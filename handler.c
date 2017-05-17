/*=============================================================================
#     FileName: handler.c
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-17 00:28:49
=============================================================================*/
#include "handler.h"
#include "threads.h"
#include "driver.h"

void *client_handler(void *thread_arg) {
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

    fprintf(stderr, "[THREAD %02d] Thread Created for Client %d\n", client_id, client_id);

    // client thread init
    char *thread_avail_flags = init_avail_flags(CLIENT_THREAD_COUNT);
    pthread_t thread_pool[CLIENT_THREAD_COUNT];

    queue_t* work_queue = NULL;
    pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

    while(1) {
        bzero(buffer, BUFFER_LEN);

        fprintf(stderr, "[THREAD %02d] Waiting for client input\n", client_id);

        if ((n = read(*newsockfd, buffer, BUFFER_LEN-1)) < 0) {
            perror("ERROR reading from socket");
            break;

        } else if(n == 0) {
            // end of file
            fprintf(stderr, "[THREAD] Client %d disconnected\n", client_id);
            break;
        }

        join_client_command(recieved_string, buffer, recv_str_len, recv_used_len);

        if(check_avail_thread(thread_avail_flags, CLIENT_COUNT) == 0) {
            fprintf(stderr, "[THREAD] Client has reached maximum thread limit. Not parsing commands until a thread becomes available.\n");
            continue;
        }

        while((cmd = get_command(recieved_string, *recv_str_len, recv_used_len)) != NULL) {
            fprintf(stderr, "[THREAD] Command: %s\n", cmd);

            int thread_id;
            while((thread_id = get_avail_thread(thread_avail_flags, CLIENT_COUNT)) == -1) {
                // this will never happen
                sleep(1);
            };

            worker_arg_t *worker_arg = malloc(sizeof(worker_arg_t));
            wrapper_arg_t *wrapper_arg = malloc(sizeof(wrapper_arg_t));

            worker_arg->newsockfd = newsockfd;
            worker_arg->command_str = cmd;
            worker_arg->client_id = client_id;
            worker_arg->work_queue = &work_queue;
            worker_arg->queue_mutex = &queue_mutex;
            worker_arg->thread_id = thread_id;
            worker_arg->thread_pool = thread_pool;
            worker_arg->pool_flag = thread_avail_flags;

            wrapper_arg->flag = thread_avail_flags + thread_id;
            wrapper_arg->worker_arg = worker_arg;

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

            } else {
                wrapper_arg->worker_func = unkn_handler;
            }

            // revert back
            if(changed)
                cmd[4] = changed;

            // wait for a thread to be available
            pthread_t *cmd_thread = thread_pool + thread_id;
            if((pthread_create(cmd_thread, NULL, handler_wrapper, (void*)wrapper_arg)) < 0) {
                perror("ERROR creating thread");
            }
        }
    }

    //flags[i] = 0;
    reset_flag(flags+client_id);
    close(*newsockfd);

    // cancel any working threads
    for(int i = 0; i < CLIENT_THREAD_COUNT; i++) {
        if(thread_avail_flags[i] == 1)
            pthread_cancel(thread_pool[i]);
    }

    return 0;
}

void *handler_wrapper(void *wrapper_arg) {
    wrapper_arg_t *arg = (wrapper_arg_t*) wrapper_arg;
    char *flag = arg->flag;

    arg->worker_func(arg->worker_arg);

    reset_flag(flag);
    free_worker_arg(arg->worker_arg);
    free(wrapper_arg);

    return 0;
}

void work_handler_cleanup(void* ptr_btches) {
    pthread_t *btches = (pthread_t*) ptr_btches;
    fprintf(stderr, "[THREAD] Cleanup\n");

    for(int i = 0; i < WORKER_COUNT; i++) {
        if(btches[i] != 0) {
            pthread_cancel(btches[i]);
            fprintf(stderr, "[THREAD] Killed workbtch %d\n", i);
        }
    }

    fprintf(stderr, "[THREAD] Finish cleanup\n");
}

void work_handler(worker_arg_t *arg) {
    int thread_id = arg->thread_id;
    int *newsockfd = arg->newsockfd;
    char *command_str = arg->command_str;
    queue_t **tid_queue = arg->work_queue;
    pthread_mutex_t *queue_mutex = arg->queue_mutex;

    while(get_tid(*tid_queue, queue_mutex) != thread_id) {
        fprintf(stderr, "[THREAD] Worker %d is waiting for its turn\n", thread_id);
        sleep(1);
    }

    fprintf(stderr, "[THREAD] Worker %d is now processing %s\n", thread_id, command_str);

    uint32_t difficulty;
    uint64_t n;
    char raw_seed[64];
    int thread_count;

    sscanf(command_str + 5, "%x %s %lx %x", &difficulty, raw_seed, &n, &thread_count);
    /*
    fprintf(stderr, "diff:%x sol:%lx cnt:%x\n", difficulty, n, thread_count);
    fprintf(stderr, "seed:");
    for(int i = 0; i < 64; i++)
        fprintf(stderr, "%c", raw_seed[i]);
    fprintf(stderr, "\n");
    */
    // n = ntohl(n);
    difficulty = ntohl(difficulty);

    uint64_t solution = 0;

    BYTE *target = get_target(difficulty);
    BYTE *seed = seed_from_raw(raw_seed);

    pthread_mutex_t sol_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_t btches[WORKER_COUNT];
    btch_arg_t btch_args[WORKER_COUNT];

    pthread_cleanup_push(work_handler_cleanup, (void*) btches);
    for(int i = 0; i < thread_count; i++) {
        btch_args[i].solution = &solution;
        btch_args[i].n = &n;
        btch_args[i].target = target;
        btch_args[i].seed = seed;
        btch_args[i].sol_mutex = &sol_mutex;

        if((pthread_create(btches + i, NULL, work_btch, (void*)(btch_args + i))) < 0) {
            perror("ERROR creating thread");
        }
        fprintf(stdout, "[THREAD] Workerbtch made %lu\n", btches[i]);
    }

    while(solution == 0) {
        fprintf(stdout, "[THREAD] Waiting for solution..\n");
        sleep(1);
    }

    fprintf(stdout, "[THREAD] Solution found! %lx\n", solution);
    fprintf(stderr, "[THREAD] Worker %d finished processing\n", thread_id);
    fprintf(stdout, "[THREAD] Finished working %s\n", command_str);

    rm_tid(tid_queue, queue_mutex);
    pthread_cleanup_pop(0);
    fprintf(stderr, "[THREAD] Worker %d exiting\n", thread_id);
    // do shit
}

void *work_btch(void *btch_arg) {
    btch_arg_t *arg = (btch_arg_t*) btch_arg;
    while(1) {
        sleep(1);
    }
}

void abrt_handler(worker_arg_t *arg) {
    // do shit
    queue_t **tid_queue = arg->work_queue;
    pthread_t *thread_pool = arg->thread_pool;
    pthread_mutex_t *queue_mutex = arg->queue_mutex;

    char *pool_flag = arg->pool_flag;
    int count = 0;
    fprintf(stderr, "[THREAD] Aborting all worker threads\n");

    while(*tid_queue != NULL) {
        int i = get_tid(*tid_queue, queue_mutex);

        pthread_cancel(thread_pool[i]);
        fprintf(stderr, "[THREAD] Killed thread %d\n", i);

        reset_flag(pool_flag + i);
        rm_tid(tid_queue, queue_mutex);
        count++;
    }
    fprintf(stderr, "[THREAD] Killed %d threads\n", count);
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
    */

    fprintf(stderr, "[THREAD] Handling SOLN\n");

    int res;
    if((res = is_valid_soln(target, seed, solution)) == -1) {
        send_message(newsockfd, "OKAY\r\n", 6);
    } else {
        fprintf(stderr, "[THREAD] Solution result %d\n", res);
        send_formatted(newsockfd, "ERRO", "Not a valid solution");
    }

    fprintf(stderr, "[THREAD] Handling SOLN Success\n");
}

void unkn_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;
    char *command_str = arg->command_str;

    fprintf(stderr, "[THREAD] Unknown command recieved: %s\n", command_str);

    send_formatted(newsockfd, "ERRO", "Unknown command");
}

void erro_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

    fprintf(stderr, "[THREAD] Handling ERRO\n");
    send_formatted(newsockfd, "ERRO", "Only I can send YOU errors =.=");
    fprintf(stderr, "[THREAD] Handling ERRO Success\n");
}

void okay_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

    fprintf(stderr, "[THREAD] Handling OKAY\n");
    send_formatted(newsockfd, "ERRO", "Dude it's not okay to send OKAY okay?");
    fprintf(stderr, "[THREAD] Handling OKAY Success\n");
}

void pong_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

    fprintf(stderr, "[THREAD] Handling PONG\n");
    send_formatted(newsockfd, "ERRO", "PONG is strictly reserved for server");
    fprintf(stderr, "[THREAD] Handling PONG Success\n");
}

void ping_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

    fprintf(stderr, "[THREAD] Handling PING\n");
    send_message(newsockfd, "PONG\r\n", 6);
    fprintf(stderr, "[THREAD] Handling PING Success\n");
}

void slep_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

    fprintf(stderr, "[THREAD] Handling SLEP\n");
    char *to_send = "sleeping 3 seconds..";
    send_formatted(newsockfd, "OKAY", to_send);

    sleep(3);

    to_send = "I just woke up!";
    send_formatted(newsockfd, "OKAY", to_send);
    fprintf(stderr, "[THREAD] Handling SLEP Success\n");
}

void free_worker_arg(worker_arg_t *arg) {
    free(arg);
}

void rm_tid(queue_t **queue, pthread_mutex_t *mutex) {
    pthread_mutex_lock(mutex);

    queue_t* tmp = *queue;
    *queue = (*queue)->next;
    free(tmp);

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

int get_tid(queue_t *queue, pthread_mutex_t *mutex) {
    int val = -1;

    pthread_mutex_lock(mutex);
    if(queue != NULL)
        val = queue->thread_id;
    pthread_mutex_unlock(mutex);

    return val;
}

