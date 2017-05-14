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
        fprintf(stderr, "[THREAD] Client %d cmds: %s", i, *recieved_string);

        fprintf(stderr, "         Breakdown: ");
        for(int i = 0; i < (int)strlen(*recieved_string); i++) {
            fprintf(stderr, "%d ", (*recieved_string)[i]);
        }
        fprintf(stderr, "\n");

        while((cmd = get_command(recieved_string)) != NULL) {
            fprintf(stderr, "Was able to get a command: %s\n", cmd);
            break;

            if(strcmp("PING\r\n", cmd) == 0) {
                ping_handler(newsockfd, cmd);
            } else {
                //to_send = "ERRO\r\n";
            }
        }
    }

    flags[i] = 0;
    close(*newsockfd);

    return 0;
}

void ping_handler(int *newsockfd, char *command_str) {
    int n;
    char *to_send = "PING";

    if ((n = write(*newsockfd, to_send, strlen(to_send))) <= 0) {
        perror("ERROR writing to socket");
    }

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

