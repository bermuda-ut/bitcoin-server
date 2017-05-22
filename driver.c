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

int main(int argc, char **argv) {
	int sockfd, portno;
	Sockaddr_in serv_addr;

	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(EXIT_FAILURE);
	}

    // open socket
	portno = atoi(argv[1]);
    make_socket(&serv_addr, &sockfd, portno);
    bind_socket(serv_addr, sockfd);

    // setup 
    init_logger(&serv_addr);
    char *thread_avail_flags = init_avail_flags(CLIENT_COUNT);
    pthread_t thread_pool[CLIENT_COUNT];

#if !DEBUG
    fclose(stdout);
#endif

    // server ready!
	listen(sockfd, CLIENT_COUNT);

    char warned = 0;
    int count = 0;
    while(1) {
        Sockaddr_in *cli_addr = malloc(sizeof(Sockaddr_in));
        socklen_t clilen = sizeof(*cli_addr);

        int *newsockfd = malloc(sizeof(int));

        fprintf(stdout, "[ SERVER ] Waiting for client\n");
        if ((*newsockfd = accept(sockfd, (Sockaddr*) cli_addr, &clilen)) < 0) {
            perror("ERROR on accept");
            //exit(EXIT_FAILURE);
        }

        // wait for a thread to become available
        int i;
        while((i = get_avail_thread(thread_avail_flags, CLIENT_COUNT)) == -1) {
            if(!warned) {
                warned = 1;
                fprintf(stdout, "[ SERVER ] Maximum client capacity reached. Waiting for someone to disconnect.\n");
            }
            sleep(1);
        };

        // reset flag
        if(warned) {
            warned = 0;
            fprintf(stdout, "[ SERVER ] Blocked connections now allowed.\n");
        }

        logger_log(cli_addr, *newsockfd, "Connected", 9);
        fprintf(stdout, "[ SERVER ] Client %d connected, new sock %d\n", i, *newsockfd);

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

        fprintf(stdout, "[ SERVER ] Served %d clients\n", ++count);
    }
	
    // clean up
	close(sockfd);
    close_logger();

	return 0; 
}

