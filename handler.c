/*=============================================================================
#     FileName: handler.c
#         Desc: handlers for client
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-22 20:33:41
=============================================================================*/
#include "handler.h"
#include "threads.h"
#include "driver.h"
#include "logger.h"

void print_queue(queue_t *q) {
    queue_t *curr = q;
    fprintf(stderr, "[ ");
    while(curr) {
        fprintf(stderr, "%d ", curr->thread_id);
        curr = curr->next;
    }
    fprintf(stderr, "]\n");
}

void *client_handler(void *thread_arg) {
    pthread_detach(pthread_self());
    thread_arg_t *args = (thread_arg_t*) thread_arg;

	char buffer[BUFFER_LEN],
        *cmd,
        **recieved_string = malloc(sizeof(char*)),
        *flags = args->flags;

    int *newsockfd = args->newsockfd,
        client_id = args->i,
        n,
        *recv_used_len = malloc(sizeof(int)),
        *recv_str_len = malloc(sizeof(int));

    *recv_str_len = INIT_CMD_STR_SIZE;
    *recv_used_len = 0;
    *recieved_string = malloc(sizeof(char) * *recv_str_len);

    //fprintf(stderr, "[THREAD %02d] Thread Created for Client %d\n", client_id, client_id);

    // client thread init
    char *thread_avail_flags = init_avail_flags(CLIENT_THREAD_COUNT);
    pthread_t *thread_pool = malloc(sizeof(pthread_t) * CLIENT_THREAD_COUNT);

    queue_t* work_queue = NULL;
    pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t worker_mutex = PTHREAD_MUTEX_INITIALIZER;
    sem_t worker_sem;
    sem_init(&worker_sem, 0, CONCURRENT_WORK_COUNT);

    while(1) {
        bzero(buffer, BUFFER_LEN);

        //fprintf(stderr, "[THREAD %02d] Waiting for client input\n", client_id);

        if ((n = read(*newsockfd, buffer, BUFFER_LEN-1)) < 0) {
            perror("ERROR reading from socket");
            break;

        } else if(n == 0) {
            // end of file
            logger_log(args->addr, *newsockfd, "Disconnected", 12);
            //fprintf(stderr, "[THREAD] Client %d disconnected\n", client_id);
            break;
        }

        join_client_command(recieved_string, buffer, recv_str_len, recv_used_len);
        fflush(stderr);

        if(check_avail_thread(thread_avail_flags, CLIENT_COUNT) == 0) {
            //fprintf(stderr, "[THREAD] Client has reached maximum thread limit. Not parsing commands until a thread becomes available.\n");
            sleep(1);
            continue;
        }

        while((cmd = get_command(recieved_string, *recv_str_len, recv_used_len)) != NULL) {
            //logger_log(NULL, 0, cmd, strlen(cmd));
            logger_log(args->addr, *newsockfd, cmd, strlen(cmd));

            int thread_id;
            while((thread_id = get_avail_thread(thread_avail_flags, CLIENT_COUNT)) == -1) {
                // this will never happen
                sleep(1);
            };
            //fprintf(stderr, "[CLIENT] T%d Command: %s\n", thread_id, cmd);

            worker_arg_t *worker_arg = malloc(sizeof(worker_arg_t));
            wrapper_arg_t *wrapper_arg = malloc(sizeof(wrapper_arg_t));

            worker_arg->newsockfd = newsockfd;
            worker_arg->client_id = client_id;
            worker_arg->thread_id = thread_id;
            worker_arg->pool_flag = thread_avail_flags;
            worker_arg->work_queue = &work_queue;
            worker_arg->worker_sem = &worker_sem;
            worker_arg->queue_mutex = &queue_mutex;
            worker_arg->command_str = cmd;
            worker_arg->thread_pool = thread_pool;
            worker_arg->worker_mutex = &worker_mutex;

            wrapper_arg->flag = thread_avail_flags + thread_id;
            wrapper_arg->worker_arg = worker_arg;

            //fprintf(stderr, "[CLIENT] T%d arg will be at %p\n", thread_id, worker_arg);
            //fprintf(stderr, "[CLIENT] T%d arg will be at %p\n", thread_id, wrapper_arg->worker_arg);

            // so that we can strcmp only first 4 characters :)
            char changed = 0;
            if(strlen(cmd) > 4 && cmd[4] == ' '){
                changed = cmd[4];
                cmd[4] = '\0';
            }

            if(strcmp("PING", cmd) == 0) {
                wrapper_arg->worker_func = ping_handler;

            } else if(strcmp("PONG", cmd) == 0) {
                wrapper_arg->worker_func = pong_handler;

            } else if(strcmp("OKAY", cmd) == 0) {
                wrapper_arg->worker_func = okay_handler;

            } else if(strcmp("ERRO", cmd) == 0) {
                wrapper_arg->worker_func = erro_handler;

            } else if(strcmp("SOLN", cmd) == 0) {
                wrapper_arg->worker_func = soln_handler;

            } else if(strcmp("SLEP", cmd) == 0) {
                wrapper_arg->worker_func = slep_handler;

            } else if(strcmp("ABRT", cmd) == 0) {
                wrapper_arg->worker_func = abrt_handler;

            } else if(strcmp("WORK", cmd) == 0) {
                wrapper_arg->worker_func = work_handler;
                push_tid(&work_queue, &queue_mutex, thread_id);
                //print_queue(work_queue);

            } else {
                wrapper_arg->worker_func = unkn_handler;
            }

            // revert back
            if(changed)
                cmd[4] = changed;

            //fprintf(stderr, "spawning thread\n");
            pthread_t *cmd_thread = thread_pool + thread_id;
            if((pthread_create(cmd_thread, NULL, handler_wrapper, (void*)wrapper_arg)) < 0) {
                perror("ERROR creating thread");
            }
        }
    }

    close(*newsockfd);

    // cancel any working threads
    for(int i = 0; i < CLIENT_THREAD_COUNT; i++) {
        //fprintf(stderr, "[THREAD] Client checking %d\n", i);
        if(thread_avail_flags[i] == 1) {
            //fprintf(stderr, "[THREAD] %d is working thread\n", i);
            pthread_cancel(thread_pool[i]);
            //pthread_join(thread_pool[i], 0);
        }
    }

    free(recv_str_len);
    free(recv_used_len);
    free(recieved_string);
    //fprintf(stderr, "[THREAD] Client thread dying\n");
    reset_flag(flags+client_id);
    return 0;
}

void *handler_wrapper(void *wrapper_arg) {
    pthread_detach(pthread_self());
    wrapper_arg_t *arg = (wrapper_arg_t*) wrapper_arg;
    char *flag = arg->flag;

    //fprintf(stderr, "[WRAPPER] Wrapper starting inner func for %p \n", arg->worker_arg);
    arg->worker_func(arg->worker_arg);

    reset_flag(flag);
    free(arg->worker_arg->command_str);
    free(arg->worker_arg);
    free(arg);
    //fprintf(stderr, "[WRAPPER] Wrapper exiting..\n");
    return 0;
}

void work_handler_cleanup(void* cleanup_arg) {
    cleanup_arg_t *arg = (cleanup_arg_t*) cleanup_arg;
    //fprintf(stderr, "[THREAD] Cleanup: %d btches remain\n", arg->thread_count);

    *(arg->cancelled) = 1;
    /*
    if(arg->btches) {
        pthread_t *btches = *(arg->btches);

        for(int i = 0; i < arg->thread_count; i++) {
            if(btches[i] != 0) {
                pthread_cancel(btches[i]);
                pthread_join(btches[i], 0);
                fprintf(stderr, "[THREAD] Killed workbtch %lu\n", btches[i]);
            }
        }
    }
    */

    queue_t **tid_queue = arg->tid_queue;
    pthread_mutex_t *mutex = arg->queue_mutex;
    int thread_id = arg->thread_id;

    //fprintf(stderr, "[THREAD] tid cleanup, thread is %d\n", thread_id);
    while(get_tid(tid_queue, mutex) != thread_id) {
        //fprintf(stderr, "Waiting for current pid \n");
        sleep(1);
    }

    rm_tid(tid_queue, mutex);

    //fprintf(stderr, "[THREAD] Finish cleanup thread %d\n", thread_id);
}

void work_handler(worker_arg_t *arg) {
    //fprintf(stderr, "[WORKER] worker spawned\n");
    int cancelled = 0;
    int thread_id = arg->thread_id;
    int *newsockfd = arg->newsockfd;
    char *command_str = arg->command_str;
    queue_t **tid_queue = arg->work_queue;
    pthread_mutex_t *queue_mutex = arg->queue_mutex;
    sem_t *worker_sem = arg->worker_sem;
    cleanup_arg_t cleanup_arg;

    cleanup_arg.tid_queue = tid_queue;
    cleanup_arg.thread_id = thread_id;
    cleanup_arg.queue_mutex = queue_mutex;
    cleanup_arg.cancelled = &cancelled;
    cleanup_arg.btches = NULL;
    cleanup_arg.thread_count = 0;
    cleanup_arg.worker_sem = worker_sem;
    pthread_cleanup_push(work_handler_cleanup, (void*)&cleanup_arg);

    uint32_t difficulty;
    uint64_t n;
    char raw_seed[64];
    int thread_count;

    sscanf(command_str + 5, "%x %s %lx %x", &difficulty, raw_seed, &n, &thread_count);
    difficulty = ntohl(difficulty);
    //n = ntohl(n);
    /*
    if(thread_count > WORKER_COUNT_MAX)
        thread_count = WORKER_COUNT_MAX;
    */

    uint64_t solution = 0;
    BYTE *target = get_target(difficulty);
    BYTE *seed = seed_from_raw(raw_seed);
    pthread_mutex_t sol_mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_t *btches = malloc(sizeof(pthread_t) * thread_count);
    btch_arg_t btch_args[thread_count];
    cleanup_arg.btches = &btches;
    cleanup_arg.thread_count = thread_count;

    //fprintf(stderr, "[THREAD] Worker %d waiting for %s\n", thread_id, command_str);
    sem_wait(worker_sem);
    //fprintf(stderr, "[THREAD] Worker %d is now processing %s\n", thread_id, command_str);

    uint64_t chunk = (UINT64_MAX - n) / thread_count;

    //fprintf(stderr, "n is %lu %lx\n", n, n);
    //fprintf(stderr, "chunk size is %lu\n", chunk);
    for(int i = 0; i < thread_count; i++) {
        btch_args[i].solution = &solution;
        btch_args[i].target = target;
        btch_args[i].seed = seed;
        btch_args[i].cancelled = &cancelled;
        btch_args[i].sol_mutex = &sol_mutex;
        btch_args[i].btch_id = i;
        btch_args[i].start = n + i * chunk;

        if(i == thread_count - 1)
            btch_args[i].end = UINT64_MAX;
        else
            btch_args[i].end = n + (i + 1) * chunk;

        if((pthread_create(btches + i, NULL, work_btch, (void*)(btch_args + i))) < 0) {
            perror("ERROR creating thread");
        }
        //fprintf(stdout, "[THREAD] Workerbtch made %lu\n", btches[i]);
    }

    while(solution == 0) {
        //fprintf(stdout, "[THREAD] Waiting for solution..\n");
        sleep(1);
    }

    int curr;
    while((curr = get_tid(tid_queue, queue_mutex)) != thread_id) {
        fprintf(stderr, "[THREAD] Worker %d is waiting for its turn, current is %d\n", thread_id, curr);
        sleep(1);
    }
    sem_post(worker_sem);

    pthread_mutex_lock(arg->worker_mutex);
    //fprintf(stdout, "[THREAD] Solution found!\n");
    char* result = malloc(sizeof(char) * 98);
    char print_seed[65];
    bzero(print_seed, 65);
    memcpy(print_seed, raw_seed, 64);
    difficulty = htonl(difficulty);
    sprintf(result, "SOLN %08x %s %016lx\r\n", difficulty, print_seed, solution);
    //fprintf(stdout, "[THREAD] Sending!\n");
    send_message(newsockfd, result, 97);
    pthread_mutex_unlock(arg->worker_mutex);

    //fprintf(stderr, "[THREAD] Worker %d cleaning up\n", thread_id);
    pthread_cleanup_pop(1);
    //fprintf(stderr, "[THREAD] Worker %d exiting\n", thread_id);
    // do shit
}

void *work_btch(void *btch_arg) {
    btch_arg_t *arg = (btch_arg_t*) btch_arg;
    BYTE* target = arg->target;
    BYTE* seed = arg->seed;
    uint64_t start = arg->start;
    uint64_t end = arg->end;
    uint64_t* solution = arg->solution;
    pthread_mutex_t* mutex = arg->sol_mutex;
    int *cancelled = arg->cancelled;

    uint64_t trying = start;
    //fprintf(stderr, "thread trying from %lu to %lu (inclusive)\n", start, end);
    while(1) {
        //fprintf(stderr, "trying %lu\n",trying);
        int res;

        if(*cancelled || trying >= end)
            break;

        if((res = is_valid_soln(target, seed, trying)) == -1) {
            //fprintf(stderr, "[WORKBTCH] Solution found %lu\n", trying);
            pthread_mutex_lock(mutex);
            *solution = trying;
            *cancelled = 1;
            break;
        //} else {
            //fprintf(stderr, "[WORKBTCH] Attempted.. Retrying in 1 second\n");
        }

        trying += 1;
    }

    //fprintf(stderr, "[WORKBTCH] Work bitch %d is now dying\n", arg->btch_id);
    return 0;
}

void abrt_handler(worker_arg_t *arg) {
    // do shit
    queue_t **tid_queue = arg->work_queue;
    pthread_t *thread_pool = arg->thread_pool;
    char *pool_flag = arg->pool_flag;
    //pthread_mutex_t *queue_mutex = arg->queue_mutex;

    //fprintf(stderr, "[THREAD] Aborting all worker threads in queue %p\n", *tid_queue);

    int count = 0;
    queue_t *curr = *tid_queue;
    while(curr) {
        pthread_cancel(thread_pool[curr->thread_id]);
        reset_flag(pool_flag + curr->thread_id);
        curr = curr->next;
        count++;
    }
    pthread_mutex_unlock(arg->worker_mutex);
}

void soln_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;
    char *command_str = arg->command_str;

    uint32_t difficulty;
    uint64_t solution;
    char raw_seed[64];
    
    sscanf(command_str + 5, "%x %s %lx", &difficulty, raw_seed, &solution);
    difficulty = ntohl(difficulty);
    //solution   = ntohl(solution);

    BYTE *target = get_target(difficulty);
    BYTE *seed = seed_from_raw(raw_seed);

    /*
    char *to_send = malloc(sizeof(char)*9999);
    fprintf(stderr, "Calculating solution for d:%u s:%lu\r\n", difficulty, solution);

    fprintf(stderr, "[THREAD] Handling SOLN\n");

    */
    int res;
    if((res = is_valid_soln(target, seed, solution)) == -1) {
        send_message(newsockfd, "OKAY\r\n", 6);
    } else {
        //fprintf(stderr, "[THREAD] Solution result %d\n", res);
        send_formatted(newsockfd, "ERRO", "Not a valid solution");
    }

    //fprintf(stderr, "[THREAD] Handling SOLN Success\n");
}

void unkn_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;
    //char *command_str = arg->command_str;

    //fprintf(stderr, "[THREAD] Unknown command recieved: %s\n", command_str);

    send_formatted(newsockfd, "ERRO", "Unknown command");
}

void erro_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

    //fprintf(stderr, "[THREAD] Handling ERRO\n");
    send_formatted(newsockfd, "ERRO", "Only I can send YOU errors =.=");
    //fprintf(stderr, "[THREAD] Handling ERRO Success\n");
}

void okay_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

    //fprintf(stderr, "[THREAD] Handling OKAY\n");
    send_formatted(newsockfd, "ERRO", "Dude it's not okay to send OKAY okay?");
    //fprintf(stderr, "[THREAD] Handling OKAY Success\n");
}

void pong_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

    //fprintf(stderr, "[THREAD] Handling PONG\n");
    send_formatted(newsockfd, "ERRO", "PONG is strictly reserved for server");
    //fprintf(stderr, "[THREAD] Handling PONG Success\n");
}

void ping_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

    //fprintf(stderr, "[THREAD] Handling PING\n");
    send_message(newsockfd, "PONG\r\n", 6);
    //fprintf(stderr, "[THREAD] Handling PING Success\n");
}

void slep_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

    //fprintf(stderr, "[THREAD] Handling SLEP\n");
    char *to_send = "sleeping 3 seconds..";
    send_formatted(newsockfd, "OKAY", to_send);

    sleep(3);

    to_send = "I just woke up!";
    send_formatted(newsockfd, "OKAY", to_send);
    //fprintf(stderr, "[THREAD] Handling SLEP Success\n");
}

void rm_tid(queue_t **queue, pthread_mutex_t *mutex) {
    pthread_mutex_lock(mutex);

    if(*queue != NULL) {
        queue_t* tmp = *queue;
        *queue = (*queue)->next;
        free(tmp);
    }

    pthread_mutex_unlock(mutex);
}

void push_tid(queue_t **queue, pthread_mutex_t *mutex, int tid) {
    pthread_mutex_lock(mutex);

    queue_t *curr = *queue;
    queue_t *new = malloc(sizeof(queue_t));

    new->next = NULL;
    new->thread_id = tid;

    if(curr != NULL) {
        while(curr->next != NULL)
            curr = curr->next;
        curr->next = new;
    } else {
        *queue = new;
    }

    pthread_mutex_unlock(mutex);
}

int get_tid(queue_t **queue, pthread_mutex_t *mutex) {
    int val = -1;

    pthread_mutex_lock(mutex);
    if(*queue != NULL)
        val = (*queue)->thread_id;
    pthread_mutex_unlock(mutex);

    return val;
}

