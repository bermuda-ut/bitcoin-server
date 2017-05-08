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

int main(int argc, char **argv) {
	char buffer[256];
	int sockfd, 
        newsockfd, 
        portno;
	Sockaddr_in serv_addr, cli_addr;

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

	socklen_t clilen = sizeof(cli_addr);
	if ((newsockfd = accept(sockfd, (Sockaddr*) &cli_addr, &clilen)) < 0) {
		perror("ERROR on accept");
		exit(EXIT_FAILURE);
    }
	
	/* Read characters from the connection,
		then process */
	bzero(buffer,256);
    int n;
	if ((n = read(newsockfd, buffer, 255)) < 0) {
		perror("ERROR reading from socket");
		exit(EXIT_FAILURE);
	}

	printf("Here is the message: %s\n",buffer);

	if ((n = write(newsockfd, "I got your message", 18)) < 0) {
		perror("ERROR writing to socket");
		exit(EXIT_FAILURE);
	}
	
	/* close socket */
	close(sockfd);
	
	return 0; 
}
