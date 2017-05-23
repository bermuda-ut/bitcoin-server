/*=============================================================================
#     FileName: driver.c
#         Desc: driver program
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-22 20:33:26
=============================================================================*/
#include "driver.h"
#include "netsock.h"
#include "handler.h"
#include "threads.h"
#include "logger.h"
#include <signal.h>

void segfault_handler(int);

int main(int argc, char **argv) {
	int sockfd, portno;
	Sockaddr_in serv_addr;

	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(EXIT_FAILURE);
	}
    signal(SIGSEGV, segfault_handler);

    // open socket
	portno = atoi(argv[1]);
    make_socket(&serv_addr, &sockfd, portno);
    bind_socket(serv_addr, sockfd);

    // setup 
    init_logger(&serv_addr);
    char *thread_avail_flags = init_avail_flags(CLIENT_COUNT);
    pthread_t thread_pool[CLIENT_COUNT];

    // server ready!
	listen(sockfd, CLIENT_COUNT);

    char warned = 0;
#if DEBUG
    int count = 0;
#endif
    while(1) {
        Sockaddr_in *cli_addr = malloc(sizeof(Sockaddr_in));
        socklen_t clilen = sizeof(*cli_addr);

        int *newsockfd = malloc(sizeof(int));

#if DEBUG
        fprintf(stderr, "[ SERVER ] Waiting for client\n");
#endif
        if ((*newsockfd = accept(sockfd, (Sockaddr*) cli_addr, &clilen)) < 0) {
            perror("ERROR on accept");
            //exit(EXIT_FAILURE);
        }

        // wait for a thread to become available
        int i;
        while((i = get_avail_thread(thread_avail_flags, CLIENT_COUNT)) == -1) {
            if(!warned) {
                warned = 1;
#if DEBUG
                fprintf(stderr, "[ SERVER ] Maximum client capacity reached. Waiting for someone to disconnect.\n");
#endif
            }
            sleep(1);
        };

        // reset flag
        if(warned) {
            warned = 0;
#if DEBUG
            fprintf(stderr, "[ SERVER ] Blocked connections now allowed.\n");
#endif
        }

        logger_log(cli_addr, *newsockfd, "Connected", 9);
#if DEBUG
        fprintf(stderr, "[ SERVER ] Client %d connected, new sock %d\n", i, *newsockfd);
#endif

        // init thread arg
        thread_arg_t *thread_arg = malloc(sizeof(thread_arg_t));
        thread_arg->newsockfd = newsockfd;
        thread_arg->flags = thread_avail_flags;
        thread_arg->i = i;
        thread_arg->addr = cli_addr;

        // create thread that is dedicated to serving the client
        if((pthread_create(thread_pool + i, NULL, client_handler, (void*)thread_arg)) < 0) {
            perror("ERROR creating thread");
        }

#if DEBUG
        fprintf(stderr, "[ SERVER ] Served %d clients\n", ++count);
#endif
    }
	
    // clean up
	close(sockfd);
    close_logger();

	return 0; 
}

void segfault_handler(int signum) {
   fprintf(stdout, "Segmentation Fault... %d :(\n", signum);
   fflush(stdout);
   perror("Error: ");

   exit(EXIT_FAILURE);
}
