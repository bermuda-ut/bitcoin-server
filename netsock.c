/*=============================================================================
#     FileName: netsock.c
#         Desc: network basics
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-22 22:45:55
=============================================================================*/
#include "netsock.h"

void make_socket(Sockaddr_in *serv_addr, int *sockfd, int portno) {
    while ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[ SOCKET ] ERROR opening socket");
        printf("[ SOCKET ] Failed to open socket. Retrying in 1 second");
        sleep(1);
    }

    bzero((char *) serv_addr, sizeof(*serv_addr));
    
    serv_addr->sin_addr.s_addr = INADDR_ANY;
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(portno);
}

void bind_socket(Sockaddr_in serv_addr, int sockfd) {
    int yes = 1;
    while (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("[ SOCKET ] ERROR set socket option");
        printf("[ SOCKET ] Failed set socket option. Retrying in 1 second");
        sleep(1);
    } 

    while (bind(sockfd, (Sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("[ SOCKET ] ERROR on binding");
        printf("Failed bind socket. Retrying in 1 second");
        sleep(1);
    }
}

