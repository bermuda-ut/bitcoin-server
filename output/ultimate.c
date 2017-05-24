
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

void read_svr(int sockfd, char* buffer);
void write_svr(int sockfd, char* buffer);
void handle(char* buffer, int sockfd, char*);

char* a = "WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 02\r\n";

char* b = "WORK 1effffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 03\r\n";

char* c = "WORK 1dffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 04\r\n";

char* d = "WORK 1d29ffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f a900000003b4ab00 04\r\n";

char* e = "WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 02\r\n";
char* f = "WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212399 01\r\n";
char* g = "WORK 1effffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 03\r\n";
char* h = "WORK 1dffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 04\r\n";

char* i = "WORK 1d29ffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000023212399 04\r\n";

char* j = "WORK 1d29ffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f a000000000000000 04\r\n";
char* l = "WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 06\r\n";
char* m = "WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 06\r\n";
char* n = "WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 06\r\n";
char* o = "WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 06\r\n";
char* k = "WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000000000000000 06\r\n";


char* p = "WORK 09ffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 0000023212399 04\r\n";

char* q = "WORK 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212399 01\r\n";

char* r = "WORK 1effffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212399 01\r\n";

char* s = "WORK 1dffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023a12399 01\r\n";

char* t = "WORK 1d29ffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f c40000000ce40000 01\r\n";

char* u = "SOLN 5fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 10000000232123a2\r\n";

char* v = "SOLN 6fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 10000000232123a2\r\n";

char* w = "SOLN 6fffffff 1231231239d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 10000000232123a2\r\n";

char* x = "SOLN\r\n";

char* y = "SOLN 1fffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023212605\r\n";

char* z = "SOLN 1effffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 100000002321ed8f\r\n";

char* aa = "SOLN 1dffffff 0000000019d6689c085ae165831e934ff763ae46a218a6c172b3f1b60a8ce26f 1000000023f6c072\r\n";

int main(int argc, char**argv) {
	int sockfd, portno;
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
	
    //handle(a, sockfd, buffer);
    //handle(b, sockfd, buffer);
    //handle(c, sockfd, buffer);
    //handle(i, sockfd, buffer);

    /*
    write_svr(sockfd, e);
    write_svr(sockfd, f);
    write_svr(sockfd, g);
    write_svr(sockfd, h);
    read_svr(sockfd, buffer);
    read_svr(sockfd, buffer);
    read_svr(sockfd, buffer);
    read_svr(sockfd, buffer);
    */

    write_svr(sockfd, j);
    write_svr(sockfd, l);
    write_svr(sockfd, m);
    write_svr(sockfd, n);
    write_svr(sockfd, o);
    write_svr(sockfd, k);
    read_svr(sockfd, buffer);
    read_svr(sockfd, buffer);
    read_svr(sockfd, buffer);
    read_svr(sockfd, buffer);
    read_svr(sockfd, buffer);
    read_svr(sockfd, buffer);
    /*

    handle(p, sockfd, buffer);
    handle(q, sockfd, buffer);
    handle(r, sockfd, buffer);
    handle(s, sockfd, buffer);
    handle(t, sockfd, buffer);
    handle(u, sockfd, buffer);
    handle(v, sockfd, buffer);
    handle(w, sockfd, buffer);
    handle(x, sockfd, buffer);
    handle(y, sockfd, buffer);
    handle(z, sockfd, buffer);
    handle(aa, sockfd, buffer);
    */

	return 0;
}

void handle(char* a, int sockfd, char *buffer) {
    write_svr(sockfd, a);
    read_svr(sockfd, buffer);
}

void write_svr(int sockfd, char* buffer) {
    printf("%s\n", buffer);

    int n = write(sockfd, buffer, strlen(buffer));

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(0);
    }
}

void read_svr(int sockfd, char* buffer) {
    bzero(buffer, 256);

    int n = read(sockfd,buffer,255);
    if (n < 0) {
        perror("ERROR reading from socket");
        exit(0);
    }
    printf("%s\n",buffer);
}
