/*=============================================================================
#     FileName: driver.c
#         Desc: driver program
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-08 19:13:50
=============================================================================*/
#include "driver.h"

void *client_handler(void *thread_arg);

int main(int argc, char **argv) {
	int sockfd, 
        portno;
	Sockaddr_in serv_addr;

	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(EXIT_FAILURE);
	}
	portno = atoi(argv[1]);

	 /* Create TCP socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("ERROR opening socket");
		exit(EXIT_FAILURE);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	/* Create address we're going to listen on (given port number)
	 - converted to network byte order & any IP address for 
	 this machine */
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);

    int yes = 1;

    // lose the pesky "Address already in use" error message
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("ERROR set socket option");
        exit(EXIT_FAILURE);
    } 

	 /* Bind address to the socket */
	if (bind(sockfd, (Sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		exit(EXIT_FAILURE);
	}
	
	/* Listen on socket - means we're ready to accept connections - 
	 incoming connection requests will be queued */
	listen(sockfd, CLIENT_COUNT);
	
	/* Accept a connection - block until a connection is ready to
	 be accepted. Get back a new file descriptor to communicate on. */

    char *thread_avail_flags = init_avail_flags();
    pthread_t thread_pool[CLIENT_COUNT * CLIENT_JOB_COUNT];

    while(1) {
        // required vars
        thread_arg_t *thread_arg = malloc(sizeof(thread_arg_t));
        Sockaddr_in *cli_addr = malloc(sizeof(Sockaddr_in));
        socklen_t clilen = sizeof(*cli_addr);
        int *newsockfd = malloc(sizeof(int));

        if ((*newsockfd = accept(sockfd, (Sockaddr*) cli_addr, &clilen)) < 0) {
            perror("ERROR on accept");
            exit(EXIT_FAILURE);
        }

        fprintf(stderr, "Client connected, newsock %d\n", *newsockfd);

        int i = get_avail_thread(thread_avail_flags);
        thread_arg->newsockfd = newsockfd;
        thread_arg->flags = thread_avail_flags;
        thread_arg->i = i;

        int res = pthread_create(thread_pool + i, NULL, client_handler, (void*)thread_arg);

        if(res < 0) {
            perror("ERROR creating thread");
        }
    }
	
	/* close socket */
	close(sockfd);
	
	return 0; 
}

char *init_avail_flags() {
    char* flags = malloc(sizeof(char) * CLIENT_COUNT * CLIENT_JOB_COUNT);
    return flags;
}

int get_avail_thread(char* flags) {
    int i = 0;
    while(i < CLIENT_COUNT * CLIENT_JOB_COUNT)
        if(flags[i] == 0)
            return i;
    return -1;
}

void *client_handler(void *thread_arg) {
    thread_arg_t *args = (thread_arg_t*) thread_arg;

	char buffer[BUFFER_LEN];
    char *flags = args->flags;
    int *newsockfd = args->newsockfd,
        i = args->i;
    
    bzero(buffer, 256);
    int n;
    if ((n = read(*newsockfd, buffer, 255)) < 0) {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    }

    printf("Here is the message: %s\n",buffer);

    if ((n = write(*newsockfd, "I got your message", 18)) < 0) {
        perror("ERROR writing to socket");
        exit(EXIT_FAILURE);
    }

    flags[i] = 0;

    return NULL;
}
