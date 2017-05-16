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

    fprintf(stderr, "[THREAD] Thread Created for Client %d\n", client_id);

    // client thread init
    char *thread_avail_flags = init_avail_flags(CLIENT_THREAD_COUNT);
    pthread_t thread_pool[CLIENT_THREAD_COUNT];

    // client worker thread init
    pthread_t worker_thread;
    queue_t **work_queue = malloc(sizeof(queue_t*)); // TODO: free this
    *work_queue = NULL;
    if((pthread_create(&worker_thread, NULL, work_manager, (void*)work_queue)) < 0) {
        perror("ERROR creating thread");
    }

    while(1) {
        bzero(buffer, BUFFER_LEN);

        fprintf(stderr, "[THREAD] Waiting for Client %d input\n", client_id);

        if ((n = read(*newsockfd, buffer, BUFFER_LEN-1)) < 0) {
            perror("ERROR reading from socket");
            break;

        } else if(n == 0) {
            // end of file
            fprintf(stderr, "[THREAD] Client %d disconnected\n", client_id);
            break;
        }

        join_client_command(recieved_string, buffer, recv_str_len, recv_used_len);

        //fprintf(stderr, "[THREAD] Client %d sent: %s", i, buffer);
        //fprintf(stderr, "[THREAD] Client %d cmds: %s\n", i, *recieved_string);

        if(check_avail_thread(thread_avail_flags, CLIENT_COUNT) == 0) {
            fprintf(stderr, "[THREAD] Client has reached maximum thread limit. Not parsing commands until a thread becomes available.\n");
            continue;
        }

        while((cmd = get_command(recieved_string, *recv_str_len, recv_used_len)) != NULL) {
            fprintf(stderr, "[THREAD] Command: %s\n", cmd);

            // so that we can strcmp only first 4 characters :)
            if(strlen(cmd) > 4 && cmd[4] == ' ')
                cmd[4] = '\0';

            worker_arg_t *worker_arg = malloc(sizeof(worker_arg_t));
            wrapper_arg_t *wrapper_arg = malloc(sizeof(wrapper_arg_t));

            worker_arg->newsockfd = newsockfd;
            worker_arg->command_str = cmd;
            worker_arg->client_id = client_id;
            worker_arg->command_len = malloc(sizeof(int));
            *(worker_arg->command_len) = strlen(cmd);

            wrapper_arg->worker_arg = worker_arg;

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

            } else if(strcmp("WORK", cmd) == 0) {
                // there is one thread alread to handle this.
                continue;

            } else {
                wrapper_arg->worker_func = unkn_handler;
            }

            // wait for a thread to be available
            int i;
            while((i = get_avail_thread(thread_avail_flags, CLIENT_COUNT)) == -1) {
                // this will never happen
            };
            wrapper_arg->flag = thread_avail_flags + i;

            pthread_t *cmd_thread = thread_pool + i;
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

void *work_manager(void* work_queue) {
    fprintf(stderr, "[WORKER] Dedicated worker created\n");
    queue_t** queue = (queue_t**) work_queue;

    while(1) {
        worker_arg_t* arg = NULL;
        while((arg = pop_queue(queue)) == NULL);

        work_btch_arg_t* btch_arg = malloc(sizeof(work_btch_arg_t));

        int *newsockfd = arg->newsockfd;
        char *command_str = arg->command_str;

        unsigned int worker_count;
        uint32_t difficulty;
        uint64_t solution,
                 answer = 0;
        char raw_seed[64];
        
        sscanf(command_str + 5, "%x %s %lx %u", &difficulty, raw_seed, &solution, &worker_count);
        fprintf(stderr, "worker info: %x %lx %u", difficulty, solution, worker_count);
        break;
        difficulty = ntohl(difficulty);

        BYTE *target = get_target(difficulty);
        BYTE *seed = seed_from_raw(raw_seed);

        btch_arg->seed = seed;
        btch_arg->target = target;
        btch_arg->n = &solution;
        btch_arg->answer = &answer;

        pthread_t btch_pool[worker_count];
        for(int i = 0; i < (int)worker_count; i++)
            if((pthread_create(btch_pool + i, NULL, work_btch, (void*)btch_arg)) < 0) {
                perror("ERROR creating thread");
            }

        while(answer == 0) {
            // not found yet
        }

        // cleanup
    }

    return 0;
}

void *work_btch(void* btch_arg) {
    work_btch_arg_t *arg = btch_arg;
    return 0;
}

worker_arg_t* pop_queue(queue_t** queue) {
    return NULL;
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
