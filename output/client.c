
/* A simple client program for a server program

   To compile: gcc client.c -o client
   				   
   To run: start the server, then run the client */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int main(int argc, char**argv)
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[9999];

	if (argc < 3) 
	{
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}

	portno = atoi(argv[2]);

	
	/* Translate host name into peer's IP address ;
	 * This is name translation service by the operating system 
	 */
	server = gethostbyname(argv[1]);
	
	if (server == NULL) 
	{
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	
	/* Building data structures for socket */

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;

	bcopy((char *)server->h_addr, 
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);

	serv_addr.sin_port = htons(portno);

	/* Create TCP socket -- active open 
	* Preliminary steps: Setup: creation of active open socket
	*/
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd < 0) 
	{
		perror("ERROR opening socket");
		exit(0);
	}
	
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
	{
		perror("ERROR connecting");
		exit(0);
	}

	/* Do processing
	*/
	


    //strcpy(buffer, "PING\r\nPING\r\n");
	
	//strcpy(buffer, "SOLN 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212147\r\n");

	
	//strcpy(buffer, "SOLN 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212605\r\n");

	    
	//strcpy(buffer, "SOLN 1effffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 100000002321ed8f\r\n");
	
	
	
    
	//strcpy(buffer, "WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212399 01\r\n");
		    
	
	//strcpy(buffer, "WORK 1effffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212399 04\r\n");
	

	//strcpy(buffer, "WORK 1dffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212399 01\r\n");
	
	/* note: large computational effort (and thus time) required  */
	    	
	//strcpy(buffer, "WORK 1d29ffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212399 04\r\n");
	
	bzero(buffer,9999);
    //strcpy(buffer, "WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 02\r\nWORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212399 01\r\nWORK 1effffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 03\r\nWORK 1dffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 04\r\nWORK 1d29ffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f a000000000000000 04\r\nWORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 06\r\nWORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 06\r\nWORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 06\r\nWORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 06\r\nWORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 06\r\n");
	strcpy(buffer, "WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212000 01\r\n");
    for(int i = 0; i < 100; i++) {
        printf("%s", buffer);

        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) {
            perror("ERROR writing to socket");
            exit(0);
        }
    }
	
    /*
    for(int i = 0; i < 100; i++) {
        bzero(buffer,9999);
        n = read(sockfd,buffer,9999);
        if (n < 0) {
            perror("ERROR reading from socket");
            exit(0);
        }
        printf("%s",buffer);
    }
    */

	return 0;
}
