/*=============================================================================
#     FileName: handler.c
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-09 14:33:22
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
    
    int n;
    char *to_send;

    fprintf(stderr, "[THREAD] Thread Created for Client %d\n", i);

    while(1) {
        bzero(buffer, BUFFER_LEN);

        if ((n = read(*newsockfd, buffer, BUFFER_LEN-1)) < 0) {
            perror("ERROR reading from socket");
            break;

        } else if(n == 0) {
            // end of file
            fprintf(stderr, "[THREAD] Client %d disconnected\n", i);
            break;
        }

        fprintf(stderr, "[THREAD] Client %d sent: ", i);
        for(int i = 0; i < BUFFER_LEN; i++) {
            fprintf(stderr, "%c", buffer[i]);
        }
        fprintf(stderr, "\n");

        if(strcmp("PING\r\n", buffer) == 0) {
            to_send = "PONG\r\n";
        } else {
            to_send = "ERRO\r\n";
        }

        if ((n = write(*newsockfd, to_send, strlen(to_send))) <= 0) {
            perror("ERROR writing to socket");
            break;
        }
    }

    flags[i] = 0;
    close(*newsockfd);

    return 0;
}

void parse_client_command(char *str) {

}

