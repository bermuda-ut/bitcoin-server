/*=============================================================================
#     FileName: logger.h
#         Desc: logger definitions
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-22 22:28:02
=============================================================================*/
#ifndef LOGGER
#define LOGGER
#include "netsock.h"
#include <time.h>

#define FILENAME "log.txt"
#define FILEMODE "w"

// from https://stackoverflow.com/questions/26423537/how-to-position-the-input-text-cursor-in-c
#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (x), (y))

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
void *print_welcome(void *file);
#endif
