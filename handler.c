/*=============================================================================
#     FileName: handler.c
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-14 21:46:54
=============================================================================*/
#include "handler.h"
#include "threads.h"
#include "driver.h"

void *client_handler(void *thread_arg) {
    thread_arg_t *args = (thread_arg_t*) thread_arg;

	char buffer[BUFFER_LEN];
    char *flags = args->flags;
    int *newsockfd = args->newsockfd,
        i = args->i;
    
    int n,
        *recv_str_len = malloc(sizeof(int));
    *recv_str_len = INIT_CMD_STR_SIZE;

    char *cmd,
        **recieved_string = malloc(sizeof(char*));
    *recieved_string = malloc(sizeof(char) * *recv_str_len);

    fprintf(stderr, "[THREAD] Thread Created for Client %d\n", i);

    char *thread_avail_flags = init_avail_flags(WORKER_COUNT);
    pthread_t thread_pool[WORKER_COUNT];

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

        join_client_command(recieved_string, buffer, recv_str_len);

        fprintf(stderr, "[THREAD] Client %d sent: %s", i, buffer);
        fprintf(stderr, "[THREAD] Client %d cmds: %s\n", i, *recieved_string);

        /*
        fprintf(stderr, "         Breakdown: ");
        for(int i = 0; i < (int)strlen(*recieved_string); i++) {
            fprintf(stderr, "%d ", (*recieved_string)[i]);
        }
        fprintf(stderr, "\n");
        */

        while((cmd = get_command(recieved_string)) != NULL) {
            fprintf(stderr, "Was able to get a command: %s\n", cmd);

            if(strlen(cmd) > 4 && cmd[4] == ' ') {
                cmd[4] = '\0';
            }

            int i;
            while((i = get_avail_thread(thread_avail_flags, CLIENT_COUNT)) == -1);

            worker_arg_t *worker_arg = malloc(sizeof(worker_arg_t));

            worker_arg->newsockfd = newsockfd;
            worker_arg->command_str = cmd;
            worker_arg->client_id = i;


            if(strcmp("PING", cmd) == 0) {
                //ping_handler(newsockfd, cmd);
                if((pthread_create(thread_pool + i, NULL, ping_handler, (void*)worker_arg)) < 0) {
                    perror("ERROR creating thread");
                }

            } else if(strcmp("PONG", cmd) == 0) {
                //pong_handler(newsockfd, cmd);
                if((pthread_create(thread_pool + i, NULL, pong_handler, (void*)worker_arg)) < 0) {
                    perror("ERROR creating thread");
                }

            } else if(strcmp("OKAY", cmd) == 0) {
                //okay_handler(newsockfd, cmd);
                if((pthread_create(thread_pool + i, NULL, okay_handler, (void*)worker_arg)) < 0) {
                    perror("ERROR creating thread");
                }

            } else if(strcmp("ERRO", cmd) == 0) {
                //erro_handler(newsockfd, cmd);
                if((pthread_create(thread_pool + i, NULL, erro_handler, (void*)worker_arg)) < 0) {
                    perror("ERROR creating thread");
                }

            } else if(strcmp("SOLN", cmd) == 0) {
                //soln_handler(newsockfd, cmd);
                if((pthread_create(thread_pool + i, NULL, soln_handler, (void*)worker_arg)) < 0) {
                    perror("ERROR creating thread");
                }

            } else if(strcmp("WORK", cmd) == 0) {
                //soln_handler(newsockfd, cmd);
                if((pthread_create(thread_pool + i, NULL, work_handler, (void*)worker_arg)) < 0) {
                    perror("ERROR creating thread");
                }

            } else {
                //unkn_handler(newsockfd, cmd);
                if((pthread_create(thread_pool + i, NULL, unkn_handler, (void*)worker_arg)) < 0) {
                    perror("ERROR creating thread");
                }
            }

        }
    }

    flags[i] = 0;
    close(*newsockfd);

    return 0;
}

void *work_handler(void *worker_arg) {
    worker_arg_t *arg = (worker_arg_t*) worker_arg;
    int *newsockfd = arg->newsockfd;
    char *command_str = arg->command_str;

    return 0;
}

void *soln_handler(void *worker_arg) {
    worker_arg_t *arg = (worker_arg_t*) worker_arg;
    int *newsockfd = arg->newsockfd;
    char *command_str = arg->command_str;

    uint32_t difficulty;
    uint64_t solution;
    char raw_seed[64];
    
    sscanf(command_str + 5, "%x %s %lx\r\n", &difficulty, raw_seed, &solution);
    difficulty = ntohl(difficulty);
    //solution   = ntohl(solution);

    BYTE *target = get_target(difficulty);
    BYTE *seed = seed_from_raw(raw_seed);

    /*
    char *to_send = malloc(sizeof(char)*9999);
    */
    fprintf(stderr, "Calculating solution for d:%u s:%lu\r\n", difficulty, solution);

    fprintf(stderr, "[THREAD] Handling SOLN\n");

    int res;
    if((res = is_valid_soln(target, seed, solution)) == -1) {
        send_message(newsockfd, "OKAY\r\n");
    } else {
        fprintf(stderr, "[THREAD] Solution result %d\n", res);
        send_message(newsockfd, "ERRO\tNot a valid solution\r\n");
    }

    fprintf(stderr, "[THREAD] Handling SOLN Success\n");

    free_worker_arg(worker_arg);
    return 0;
}

void *unkn_handler(void *worker_arg) {
    worker_arg_t *arg = (worker_arg_t*) worker_arg;
    int *newsockfd = arg->newsockfd;

    char to_send[45] = "ERRO\tUnknown command.\r\n";
    send_message(newsockfd, to_send);

    free_worker_arg(worker_arg);
    return 0;
}

void *erro_handler(void *worker_arg) {
    worker_arg_t *arg = (worker_arg_t*) worker_arg;
    int *newsockfd = arg->newsockfd;

    char to_send[45] = "ERRO\tThis is used to send YOU errors =.=\r\n";
    fprintf(stderr, "[THREAD] Handling ERRO\n");
    send_message(newsockfd, to_send);
    fprintf(stderr, "[THREAD] Handling ERRO Success\n");

    free_worker_arg(worker_arg);
    return 0;
}

void *okay_handler(void *worker_arg) {
    worker_arg_t *arg = (worker_arg_t*) worker_arg;
    int *newsockfd = arg->newsockfd;

    char to_send[45] = "ERRO\tDude it's not okay to send OKAY okay?\r\n";
    fprintf(stderr, "[THREAD] Handling OKAY\n");
    send_message(newsockfd, to_send);
    fprintf(stderr, "[THREAD] Handling OKAY Success\n");

    free_worker_arg(worker_arg);
    return 0;
}

void *pong_handler(void *worker_arg) {
    worker_arg_t *arg = (worker_arg_t*) worker_arg;
    int *newsockfd = arg->newsockfd;

    char to_send[45] = "ERRO\tPONG is strictly reserved for server\r\n";
    fprintf(stderr, "[THREAD] Handling PONG\n");
    send_message(newsockfd, to_send);
    fprintf(stderr, "[THREAD] Handling PONG Success\n");

    free_worker_arg(worker_arg);
    return 0;
}

void *ping_handler(void *worker_arg) {
    worker_arg_t *arg = (worker_arg_t*) worker_arg;
    int *newsockfd = arg->newsockfd;

    fprintf(stderr, "[THREAD] Handling PING\n");
    send_message(newsockfd, "PONG\r\n");
    fprintf(stderr, "[THREAD] Handling PING Success\n");

    free_worker_arg(worker_arg);
    return 0;
}

void free_worker_arg(worker_arg_t *arg) {
    free(arg);
}
