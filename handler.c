/*=============================================================================
#     FileName: handler.c
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-16 14:14:10
=============================================================================*/
#include "handler.h"
#include "driver.h"
#include "threads.h"

void send_message(int *newsockfd, char* to_send, pthread_mutex_t *mutex);
void send_formatted(int *newsockfd, char* info, char* msg, pthread_mutex_t *mutex);

/*
 * Client handler
 * */
void *client_handler(void *thread_arg) {
    thread_arg_t *args = (thread_arg_t*) thread_arg; //TODO: free this

	char buffer[BUFFER_LEN],
        *cmd,
        **recieved_string = malloc(sizeof(char*)),
        *flags = args->flags;

    int client_id = args->i,
        n,
        *newsockfd = args->newsockfd,
        *recv_used_len = malloc(sizeof(int)),
        *recv_str_len = malloc(sizeof(int));

    *recv_str_len = INIT_CMD_STR_SIZE;
    *recv_used_len = 0;
    *recieved_string = malloc(sizeof(char) * *recv_str_len);
    pthread_mutex_t send_msg_mutex = PTHREAD_MUTEX_INITIALIZER;

    fprintf(stderr, "[THREAD] Thread Created for Client %d\n", client_id);

    // init worker thread
    work_queue_t **queue = malloc(sizeof(work_queue_t*));

    // init command threads
    pthread_t thread_pool[CLIENT_THREAD_COUNT + 1]; // last thread is for the worker
    char *thread_avail_flags = init_avail_flags(CLIENT_THREAD_COUNT);

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

        if(check_avail_thread(thread_avail_flags, CLIENT_THREAD_COUNT) == 0) {
            fprintf(stderr, "[THREAD] Client has reached maximum thread limit. Not parsing commands until a thread becomes available.\n");
            continue;
        }

        while((cmd = get_command(recieved_string, *recv_str_len, recv_used_len)) != NULL) {
            fprintf(stderr, "[THREAD] Command: %s\n", cmd);

            // so that we can strcmp only first 4 characters :)
            if(strlen(cmd) > 4 && cmd[4] == ' ')
                cmd[4] = '\0';

            // wait for a thread to be available
            int i;
            if(strcmp("WORK", cmd) == 0) {
                i = CLIENT_THREAD_COUNT; // last, unreachable one
            } else {
                while((i = get_avail_thread(thread_avail_flags, CLIENT_THREAD_COUNT)) == -1) {
                    // this should never happen
                };
            }

            worker_arg_t *worker_arg = malloc(sizeof(worker_arg_t));
            wrapper_arg_t *wrapper_arg = malloc(sizeof(wrapper_arg_t));

            worker_arg->newsockfd = newsockfd;
            worker_arg->client_id = args->i;
            worker_arg->command_str = cmd;
            worker_arg->send_msg_mutex = &send_msg_mutex;
            worker_arg->command_len = malloc(sizeof(int));
            *(worker_arg->command_len) = strlen(cmd);

            wrapper_arg->flag = thread_avail_flags + i;
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
                wrapper_arg->worker_func = work_handler;

            } else {
                wrapper_arg->worker_func = unkn_handler;
            }

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
    for(int i = 0; i < CLIENT_THREAD_COUNT + 1; i++) {
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

void work_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;
    char *command_str = arg->command_str;
    fprintf(stderr, "[THREAD] Handling WORK\n");
    fprintf(stderr, "[THREAD] Handling WORK Success\n");
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
    pthread_mutex_t *msg_mutex = arg->send_msg_mutex;
    if((res = is_valid_soln(target, seed, solution)) == -1) {
        send_message(newsockfd, "OKAY\r\n", msg_mutex);
    } else {
        fprintf(stderr, "[THREAD] Solution result %d\n", res);
        send_formatted(newsockfd, "ERRO", "Not a valid solution", msg_mutex);
    }

    fprintf(stderr, "[THREAD] Handling SOLN Success\n");
}

void unkn_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;
    char *command_str = arg->command_str;
    pthread_mutex_t *msg_mutex = arg->send_msg_mutex;

    fprintf(stderr, "[THREAD] Unknown command recieved: %s\n", command_str);

    send_formatted(newsockfd, "ERRO", "Unknown command", msg_mutex);
}

void erro_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;
    pthread_mutex_t *msg_mutex = arg->send_msg_mutex;

    fprintf(stderr, "[THREAD] Handling ERRO\n");
    send_formatted(newsockfd, "ERRO", "Only I can send YOU errors =.=", msg_mutex);
    fprintf(stderr, "[THREAD] Handling ERRO Success\n");
}

void okay_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

    pthread_mutex_t *msg_mutex = arg->send_msg_mutex;
    fprintf(stderr, "[THREAD] Handling OKAY\n");
    send_formatted(newsockfd, "ERRO", "Dude it's not okay to send OKAY okay?", msg_mutex);
    fprintf(stderr, "[THREAD] Handling OKAY Success\n");
}

void pong_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

    pthread_mutex_t *msg_mutex = arg->send_msg_mutex;
    fprintf(stderr, "[THREAD] Handling PONG\n");
    send_formatted(newsockfd, "ERRO", "PONG is strictly reserved for server", msg_mutex);
    fprintf(stderr, "[THREAD] Handling PONG Success\n");
}

void ping_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;

    pthread_mutex_t *msg_mutex = arg->send_msg_mutex;
    fprintf(stderr, "[THREAD] Handling PING\n");
    send_message(newsockfd, "PONG\r\n", msg_mutex);
    fprintf(stderr, "[THREAD] Handling PING Success\n");
}

void slep_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;
    pthread_mutex_t *msg_mutex = arg->send_msg_mutex;

    fprintf(stderr, "[THREAD] Handling SLEP\n");
    char *to_send = "sleeping 3 seconds..";
    send_formatted(newsockfd, "OKAY", to_send, msg_mutex);

    sleep(3);

    to_send = "I just woke up!";
    send_formatted(newsockfd, "OKAY", to_send, msg_mutex);
    fprintf(stderr, "[THREAD] Handling SLEP Success\n");
}

// etc helpers

void free_worker_arg(worker_arg_t *arg) {
    if(arg) {
        if(arg->command_len) free(arg->command_len);
        if(arg->command_str) free(arg->command_str);
        free(arg);
    }
}

void send_formatted(int *newsockfd, char* info, char* msg, pthread_mutex_t* mutex) {
    char *to_send = malloc(sizeof(char) * 45);

    memcpy(to_send, info, 4);
    to_send[4] = '\t';

    for(int i = 0; i < 40; i++) {
        to_send[5+i] = 32;
    }

    if(msg) {
        int len = strlen(msg);
        int i = len;

        if(i > 40)
            i = 40;

        memcpy(to_send+5, msg, i);
    }

    to_send[43] = '\r';
    to_send[44] = '\n';

    send_message(newsockfd, to_send, mutex);
}

void send_message(int *newsockfd, char* to_send, pthread_mutex_t *mutex) {
    int n;

    pthread_mutex_lock (mutex);
    if ((n = write(*newsockfd, to_send, strlen(to_send))) <= 0) {
        perror("ERROR writing to socket");
    }
    pthread_mutex_unlock (mutex);
}
