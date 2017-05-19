/*=============================================================================
#     FileName: logger.h
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-19 02:20:31
#      History:
=============================================================================*/
#ifndef LOGGER
#define LOGGER
#include "netsock.h"
#include <time.h>

typedef struct {
    Sockaddr_in *src;
    struct tm *timeinfo;
    char* str;
    int len;
    int id;
} logger_arg_t;

extern int init_logger(Sockaddr_in *);
extern void close_logger();
extern void logger_log(Sockaddr_in* src, int id, char* str, int len);
void *logger_handler(void* logger_arg);
#endif
