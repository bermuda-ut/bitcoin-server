/*=============================================================================
#     FileName: handler.c
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-14 13:38:26
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

            if(strcmp("PING", cmd) == 0) {
                ping_handler(newsockfd, cmd);
            } else if(strcmp("PONG", cmd) == 0) {
                pong_handler(newsockfd, cmd);
            } else if(strcmp("OKAY", cmd) == 0) {
                okay_handler(newsockfd, cmd);
            } else if(strcmp("ERRO", cmd) == 0) {
                erro_handler(newsockfd, cmd);
            } else {
                //to_send = "ERRO\r\n";
            }
        }
    }

    flags[i] = 0;
    close(*newsockfd);

    return 0;
}

void erro_handler(int *newsockfd, char *command_str) {
    char to_send[47] = "ERRO\tThis is strictly used to send YOU errors\r\n";
    fprintf(stderr, "[THREAD] Handling ERRO\n");
    send_message(newsockfd, to_send);
    fprintf(stderr, "[THREAD] Handling ERRO Success\n");

    free(command_str);
}

void okay_handler(int *newsockfd, char *command_str) {
    char to_send[47] = "ERRO\tDude it's not okay to send OKAY okay?\r\n";
    fprintf(stderr, "[THREAD] Handling OKAY\n");
    send_message(newsockfd, to_send);
    fprintf(stderr, "[THREAD] Handling OKAY Success\n");

    free(command_str);
}

void pong_handler(int *newsockfd, char *command_str) {
    char to_send[47] = "ERRO\tPONG is strictly reserved for server\r\n";
    fprintf(stderr, "[THREAD] Handling PONG\n");
    send_message(newsockfd, to_send);
    fprintf(stderr, "[THREAD] Handling PONG Success\n");

    free(command_str);
}

void ping_handler(int *newsockfd, char *command_str) {
    fprintf(stderr, "[THREAD] Handling PING\n");
    send_message(newsockfd, "PONG\r\n");
    fprintf(stderr, "[THREAD] Handling PING Success\n");

    free(command_str);
}

char* get_command(char** full_cmd_str) {
    int len = strlen(*full_cmd_str);

    for(int i = 0; i < len - 1; i++) {
        if((*full_cmd_str)[i] == '\r' && (*full_cmd_str)[i+1] == '\n') {
            // 0 1 2 3 4  5  6 7 8 9
            // P I N G \r \n P I N G
            //          ^ i is here

            char *cmd = malloc(sizeof(char) * (i + 1));

            for(int j = 0; j < i; j++) {
                cmd[j] = (*full_cmd_str)[j];
            }
            cmd[i] = '\0';

            char *rest = malloc(sizeof(char) * len);
            strcpy(rest, (*full_cmd_str) + i + 2);

            free(*full_cmd_str);
            *full_cmd_str = rest;

            return cmd;
        }
    }

    return NULL;
}

void join_client_command(char **str, char *command_str, int *str_len) {
    int currlen = strlen(*str);

    if(currlen > *str_len - 3) {
        *str_len = *str_len * 2;
        str = realloc(str, sizeof(char) * *str_len);
    }

    strcat(*str, command_str);
}

void send_message(int *newsockfd, char* to_send) {
    int n;
    if ((n = write(*newsockfd, to_send, strlen(to_send))) <= 0) {
        perror("ERROR writing to socket");
    }
}


