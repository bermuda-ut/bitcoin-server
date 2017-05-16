/*=============================================================================
#     FileName: driver.c
#         Desc: driver program
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-09 09:18:33
=============================================================================*/
#include "driver.h"
#include "netsock.h"
#include "handler.h"
#include "threads.h"

int main(int argc, char **argv) {
	int sockfd, portno;
	Sockaddr_in serv_addr;

	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(EXIT_FAILURE);
	}
	portno = atoi(argv[1]);

    make_socket(&serv_addr, &sockfd, portno);
    bind_socket(serv_addr, sockfd);

    char *thread_avail_flags = init_avail_flags(CLIENT_COUNT);
    pthread_t thread_pool[CLIENT_COUNT];

	listen(sockfd, CLIENT_COUNT);

    while(1) {
        thread_arg_t *thread_arg = malloc(sizeof(thread_arg_t));
        Sockaddr_in *cli_addr = malloc(sizeof(Sockaddr_in));
        socklen_t clilen = sizeof(*cli_addr);

        int *newsockfd = malloc(sizeof(int));

        fprintf(stderr, "[SERVER] Waiting for client\n");
        if ((*newsockfd = accept(sockfd, (Sockaddr*) cli_addr, &clilen)) < 0) {
            perror("ERROR on accept");
            exit(EXIT_FAILURE);
        }

        // wait for a thread to become available
        int i;
        while((i = get_avail_thread(thread_avail_flags, CLIENT_COUNT)) == -1) {
            fprintf(stderr, "[SERVER] Maximum client capacity reached. Waiting for someone to disconnect.\n");
        };

        thread_arg->newsockfd = newsockfd;
        thread_arg->flags = thread_avail_flags;
        thread_arg->i = i;

        fprintf(stderr, "[SERVER] Client %d connected, new sock %d\n", i, *newsockfd);

        if((pthread_create(thread_pool + i, NULL, client_handler, (void*)thread_arg)) < 0) {
            perror("ERROR creating thread");
        }
    }
	
	/* close socket */
	close(sockfd);
	
	return 0; 
}

