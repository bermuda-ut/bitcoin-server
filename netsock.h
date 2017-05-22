/*=============================================================================
#     FileName: netsock.h
#         Desc: network basics
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-22 22:45:44
=============================================================================*/
#ifndef NETSOCK
#define NETSOCK

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "driver.h"

typedef struct sockaddr_in Sockaddr_in;
typedef struct sockaddr Sockaddr;

extern void make_socket(Sockaddr_in *serv_addr, int *sockfd, int portno);
extern void bind_socket(Sockaddr_in serv_addr, int sockfd);

#endif
