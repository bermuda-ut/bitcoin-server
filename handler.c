/*=============================================================================
#     FileName: handler.c
#         Desc: handlers for client
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-22 22:27:46
=============================================================================*/
#include "handler.h"
#include "threads.h"
#include "driver.h"
#include "logger.h"

/**
 * Client handling thread function
 * Get client command -> spawn thread to handle that command
 */
void *client_handler(void *thread_arg) {
    // so that server does not need to join later
    pthread_detach(pthread_self());

    // init
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

#if DEBUG
    fprintf(stderr, "[ CLIENT %02d ] Thread Created for Client %d\n", client_id, client_id);
#endif

    // client thread init
    char *thread_avail_flags = init_avail_flags(CLIENT_THREAD_COUNT);
    pthread_t thread_pool[CLIENT_THREAD_COUNT];// = malloc(sizeof(pthread_t) * CLIENT_THREAD_COUNT);

    queue_t* work_queue = NULL;
    pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t worker_mutex = PTHREAD_MUTEX_INITIALIZER;
    sem_t worker_sem;
    sem_init(&worker_sem, 0, CONCURRENT_WORK_COUNT);

    while(1) {
        bzero(buffer, BUFFER_LEN);
#if DEBUG
        fprintf(stderr, "[ CLIENT %02d ] Waiting for client input\n", client_id);
#endif

        if ((n = read(*newsockfd, buffer, BUFFER_LEN-1)) < 0) {
            perror("ERROR reading from socket");
            break;

        } else if(n == 0) {
            // end of file
            logger_log(args->addr, *newsockfd, "Disconnected", 12);
#if DEBUG
            fprintf(stderr, "[ CLIENT %02d ] Client disconnected\n", client_id);
#endif
            break;
        }

        join_client_command(recieved_string, buffer, recv_str_len, recv_used_len);

        if(check_avail_thread(thread_avail_flags, CLIENT_COUNT) == 0) {
#if DEBUG
            fprintf(stderr, "[ CLIENT %02d ] Client has reached maximum thread limit. Not parsing commands until a thread becomes available.\n", client_id);
#endif
            sleep(1);
            continue;
        }

        while((cmd = get_command(recieved_string, *recv_str_len, recv_used_len)) != NULL) {
            logger_log(args->addr, *newsockfd, cmd, strlen(cmd));

            int thread_id;

            while((thread_id = get_avail_thread(thread_avail_flags, CLIENT_COUNT)) == -1) {
                // this will never happen
                sleep(1);
            };

#if DEBUG
            fprintf(stderr, "[ CLIENT ] Thread: %d\tCommand: %s\n", thread_id, cmd);
#endif

            // init thread args
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

            // so that we can strcmp only first 4 characters :)
            char changed = 0;
            if(strlen(cmd) > 4 && cmd[4] == ' '){
                changed = cmd[4];
                cmd[4] = '\0';
            }

            // get the right handler
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

            // revert back that 4th character trick
            if(changed)
                cmd[4] = changed;

            // spawn handler in wrapper
            pthread_t *cmd_thread = thread_pool + thread_id;
#if DEBUG
            fprintf(stderr, "[ CLIENT %02d ] thread making at %d cleaning up..\n", client_id, thread_id);
#endif
            if((pthread_create(cmd_thread, NULL, handler_wrapper, (void*)wrapper_arg)) < 0) {
                perror("ERROR creating thread");
            }
        }
    }

    // clean up :)

    close(*newsockfd);

#if DEBUG
    fprintf(stderr, "[ CLIENT %02d ] Client thread cleaning up..\n", client_id);
#endif
    for(int i = 0; i < CLIENT_THREAD_COUNT; i++)
        if(thread_avail_flags[i] == 1)
            pthread_cancel(thread_pool[i]);

    free(recv_str_len);
    free(recv_used_len);
    free(recieved_string);

#if DEBUG
    fprintf(stderr, "[ CLIENT %02d ] Client thread dying\n", client_id);
#endif
    reset_flag(flags+client_id);

    return 0;
}

/**
 * Wrapper thread for all the handlers. Does all the init and
 * cleanups
 */
void *handler_wrapper(void *wrapper_arg) {
    pthread_detach(pthread_self());
    wrapper_arg_t *arg = (wrapper_arg_t*) wrapper_arg;
    char *flag = arg->flag;

    arg->worker_func(arg->worker_arg);

    reset_flag(flag);
    free(arg->worker_arg->command_str);
    free(arg->worker_arg);
    free(arg);

    return 0;
}

/**
 * Kill all worker threads
 */
void abrt_handler(worker_arg_t *arg) {
    queue_t **tid_queue = arg->work_queue;
    pthread_t *thread_pool = arg->thread_pool;
    char *pool_flag = arg->pool_flag;
    //pthread_mutex_t *queue_mutex = arg->queue_mutex;

#if DEBUG
    fprintf(stderr, "[ ABORT ] Aborting all worker threads for client %02d\n", arg->client_id);
#endif

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

/**
 * Check if valid solution
 */
void soln_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;
    char *command_str = arg->command_str;

#if DEBUG
    fprintf(stderr, "[ SOLUTION ] Init for client %02d\n", arg->client_id);
#endif

    uint32_t difficulty;
    uint64_t solution;
    char raw_seed[64];
    
    sscanf(command_str + 5, "%x %s %lx", &difficulty, raw_seed, &solution);
    difficulty = ntohl(difficulty);

    BYTE *target = get_target(difficulty);
    BYTE *seed = seed_from_raw(raw_seed);

    int res;
    if((res = is_valid_soln(target, seed, solution)) == -1) {
        send_message(newsockfd, "OKAY\r\n", 6);
    } else {
        send_formatted(newsockfd, "ERRO", "Not a valid solution");
    }

#if DEBUG
    fprintf(stderr, "[ SOLUTION ] SOLN result %d for client %02d\n", res, arg->client_id);
#endif
}

/**
 * Basic handlers
 */

void unkn_handler(worker_arg_t *arg) {
#if DEBUG
    fprintf(stderr, "[ THREAD ] Unknown command from %02d. Recieved: %s\n", arg->client_id, arg->command_str);
#endif
    send_formatted(arg->newsockfd, "ERRO", "Unknown command");
}

void erro_handler(worker_arg_t *arg) {
#if DEBUG
    fprintf(stderr, "[ THREAD ] Handling ERRO for %02d\n", arg->client_id);
#endif
    send_formatted(arg->newsockfd, "ERRO", "Only I can send YOU errors =.=");
}

void okay_handler(worker_arg_t *arg) {
#if DEBUG
    fprintf(stderr, "[ THREAD ] Handling OKAY for %02d\n", arg->client_id);
#endif
    send_formatted(arg->newsockfd, "ERRO", "Dude it's not okay to send OKAY okay?");
}

void pong_handler(worker_arg_t *arg) {
#if DEBUG
    fprintf(stderr, "[ THREAD ] Handling PONG for %02d\n", arg->client_id);
#endif
    send_formatted(arg->newsockfd, "ERRO", "PONG is strictly reserved for server");
}

void ping_handler(worker_arg_t *arg) {
#if DEBUG
    fprintf(stderr, "[ THREAD ] Handling PING for %02d\n", arg->client_id);
#endif
    send_message(arg->newsockfd, "PONG\r\n", 6);
}

void slep_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

#if DEBUG
    fprintf(stderr, "[ THREAD ] Handling SLEP for %02d\n", arg->client_id);
#endif
    char *to_send = "sleeping 3 seconds..";
    send_formatted(newsockfd, "OKAY", to_send);

    // good night
    sleep(3);

    to_send = "I just woke up!";
    send_formatted(newsockfd, "OKAY", to_send);
}

