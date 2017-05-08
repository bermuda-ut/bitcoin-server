/*=============================================================================
#     FileName: driver.h
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-08 19:39:22
=============================================================================*/
#ifndef DRIVER
#define DRIVER

#define CLIENT_COUNT 100
#define CLIENT_JOB_COUNT 10
#define BUFFER_LEN 256

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>
#include <semaphore.h>

typedef struct {
    int* newsockfd;
    char* flags;
    int i;
} thread_arg_t;

typedef struct sockaddr_in Sockaddr_in;
typedef struct sockaddr Sockaddr;

char *init_avail_flags();
int get_avail_thread(char* flags);

#endif
