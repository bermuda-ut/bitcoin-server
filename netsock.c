/*=============================================================================
#     FileName: netsock.c
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-09 09:32:40
#      History:
=============================================================================*/
#include "netsock.h"

void make_socket(Sockaddr_in *serv_addr, int *sockfd, int portno) {
    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    bzero((char *) serv_addr, sizeof(*serv_addr));
    
    serv_addr->sin_addr.s_addr = INADDR_ANY;
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(portno);
}

void bind_socket(Sockaddr_in serv_addr, int sockfd) {
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("ERROR set socket option");
        exit(EXIT_FAILURE);
    } 

    if (bind(sockfd, (Sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(EXIT_FAILURE);
    }
}

