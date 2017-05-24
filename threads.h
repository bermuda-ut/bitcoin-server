/*=============================================================================
#     FileName: threads.h
#         Desc: thread basics
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-09 14:33:35
=============================================================================*/
#include <pthread.h>
#include <semaphore.h>
#include "driver.h"
#include "netsock.h"

typedef struct {
    int* newsockfd;
    char* flags;
    int i;
    pthread_mutex_t *client_pool_mutex;
	Sockaddr_in *addr;
} thread_arg_t;

extern char *init_avail_flags(int len);
extern int get_avail_thread(char* flags, int count, pthread_mutex_t* mutex);
extern int check_avail_thread(char* flags, int count, pthread_mutex_t* mutex);
extern void reset_flag(char* flag, pthread_mutex_t* mutex);
