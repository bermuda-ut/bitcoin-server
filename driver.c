/*=============================================================================
#     FileName: driver.c
#         Desc: driver program
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-25 22:10:28
=============================================================================*/
#include "driver.h"
#include "netsock.h"
#include "handler.h"
#include "threads.h"
#include "logger.h"
#include <signal.h>

/**
 * Good-enough for project submission server
 *
 * TODO for production:
 *
 *  - Balancer that spawns multiple remote instances of the server process
 *    then balances the workload
 *    This would allow us to handle more than CLIENT_COUNT and balance
 *    clients according to number of WORK commands
 *
 *  - Proper logger that will not die when terminated
 *    Have a logger thread which will not die until it finishes logging
 *    Need to catch signal SIGTERM and SIGINT, kill all other threads,
 *    wait for logger threads, then exit()
 *
 *  - Find and free all memory leaks :P
 *
 *
 * DONE:
 *  - Under mega stress load (10k clients trying to connect),
 *    for some reason, client threads are 'stuck' at read() 
 *    and we have processed the client command 
 *    but client is waiting for our message.
 *    No clue why this happens.
 *    This does not happen <100 clients.
 *     > Before, sever queued pending clients.
 *     > Now server just disconnects them so this issue never occurs :)
 *     > happened. FIXED
 *
 *  - SIGPIPE handling
 *     > ignore them >:)
 */

int global_work_count = 0;
int main(int argc, char **argv) {
	int sockfd, portno;
	Sockaddr_in serv_addr;

	if (argc < 2) {
		fprintf(stderr, "[SERVER ] ERROR, no port provided\n");
		exit(EXIT_FAILURE);
	}

    // custom signal handlers
    signal(SIGSEGV, segfault_handler);
    signal(SIGPIPE, SIG_IGN);

    // open socket
	portno = atoi(argv[1]);
    make_socket(&serv_addr, &sockfd, portno);
    bind_socket(serv_addr, sockfd);

    // setup 
    init_logger(&serv_addr);
    char *thread_avail_flags = init_avail_flags(CLIENT_COUNT);
    pthread_t thread_pool[CLIENT_COUNT];
    bzero(thread_pool, sizeof(pthread_t) * CLIENT_COUNT);
    pthread_mutex_t client_pool_mutex = PTHREAD_MUTEX_INITIALIZER;

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
#if DEBUG
            perror("[ SERVER ] ERROR on accept");
#endif
            free(newsockfd);
            free(cli_addr);
            continue;
        }

#if DEBUG
        fprintf(stderr, "[ SERVER ] Available client slots: %d\n", count_avail_thread(thread_avail_flags, CLIENT_COUNT, &client_pool_mutex));
        fflush(stderr);
#endif

        // Wait for a thread to become available
        int i;
        if((i = get_avail_thread(thread_avail_flags, CLIENT_COUNT, &client_pool_mutex)) == -1) {
            if(!warned) {
                warned = 1;
#if DEBUG
                fprintf(stderr, "[ SERVER ] Maximum client capacity reached. Refusing connections.\n");
#endif
            }
            char* to_log = "Maximum number of clients reached. Disconnecting new client.";
            logger_log(cli_addr, *newsockfd, to_log, strlen(to_log));

            close(*newsockfd);
            free(newsockfd);
            free(cli_addr);
            continue;
        };

        // join the dead thread
        if(thread_pool[i]) {
            //pthread_cancel(thread_pool[i]);
            pthread_join(thread_pool[i], NULL);
        }

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
        thread_arg->client_pool_mutex = &client_pool_mutex;

        // create thread that is dedicated to serving the client
        if((pthread_create(thread_pool + i, NULL, client_handler, (void*)thread_arg)) < 0) {
#if DEBUG
            perror("[ SERVER ] ERROR creating thread");
#endif
            close(*newsockfd);
            free(thread_arg);
            free(newsockfd);
            free(cli_addr);
            continue;
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
   fprintf(stdout, "Segmentation Fault (%d)\n  </3  \nGood bye..\n", signum);
   fflush(stdout);
   exit(EXIT_FAILURE);
}
