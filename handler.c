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
#include "threads.h"
#include "driver.h"

/*
 * Client handler
 * */
void *client_handler(void *thread_arg) {
    thread_arg_t *args = (thread_arg_t*) thread_arg;

	char buffer[BUFFER_LEN];
    char *flags = args->flags;
    int *newsockfd = args->newsockfd,
        i = args->i;
    
    int n,
        *recv_used_len = malloc(sizeof(int)),
        *recv_str_len = malloc(sizeof(int));
    *recv_str_len = INIT_CMD_STR_SIZE;
    *recv_used_len = 0;

    char *cmd,
        **recieved_string = malloc(sizeof(char*));
    *recieved_string = malloc(sizeof(char) * *recv_str_len);

    fprintf(stderr, "[THREAD] Thread Created for Client %d\n", i);

    char *thread_avail_flags = init_avail_flags(CLIENT_THREAD_COUNT);
    pthread_t thread_pool[CLIENT_THREAD_COUNT];

    while(1) {
        bzero(buffer, BUFFER_LEN);

        fprintf(stderr, "[THREAD] Waiting for Client %d input\n", i);

        if ((n = read(*newsockfd, buffer, BUFFER_LEN-1)) < 0) {
            perror("ERROR reading from socket");
            break;

        } else if(n == 0) {
            // end of file
            fprintf(stderr, "[THREAD] Client %d disconnected\n", i);
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
            fprintf(stderr, "[THREAD] Command: ");
            for(int i = 0; i < strlen(cmd); i++)
                fprintf(stderr, "%c", cmd[i]);
            fprintf(stderr, "\n");

            // so that we can strcmp only first 4 characters :)
            if(strlen(cmd) > 4 && cmd[4] == ' ')
                cmd[4] = '\0';

            // wait for a thread to be available
            int i;
            while((i = get_avail_thread(thread_avail_flags, CLIENT_COUNT)) == -1) {
                // do I need to save state here?
            };

            worker_arg_t *worker_arg = malloc(sizeof(worker_arg_t));
            wrapper_arg_t *wrapper_arg = malloc(sizeof(wrapper_arg_t));

            worker_arg->newsockfd = newsockfd;
            worker_arg->command_str = cmd;
            worker_arg->client_id = i;
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
    reset_flag(flags+i);
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
    if((res = is_valid_soln(target, seed, solution)) == -1) {
        send_message(newsockfd, "OKAY\r\n");
    } else {
        fprintf(stderr, "[THREAD] Solution result %d\n", res);
        send_formatted(newsockfd, "ERRO", "Not a valid solution");
    }

    fprintf(stderr, "[THREAD] Handling SOLN Success\n");
}

void unkn_handler(worker_arg_t *arg) {
    int *newsockfd = arg->newsockfd;
    char *command_str = arg->command_str;

    fprintf(stdout, "[THREAD] Unknown command recieved: %s\n", command_str);

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
    send_message(newsockfd, "PONG\r\n");
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
