/*=============================================================================
#     FileName: driver.h
#         Desc: global header
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-24 18:14:33
=============================================================================*/
#ifndef DRIVER
#define DRIVER

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define DEBUG 0
#define LOG_OUTPUT 0
#define LOG_ONLY_CLIENT 0

#define CLIENT_COUNT 100
#define BUFFER_LEN 256

void segfault_handler(int);
void sigabrt_handler(int);

// to meet the spec
// and probably a good idea to not overload the server.. :)
#define GLOBAL_WORK_LIMIT 10
extern int global_work_count;
extern pthread_mutex_t global_work_mutex;

// for logger purpose
extern int curr_cli_count;
extern int global_served_count;

#endif
