/*=============================================================================
#     FileName: logger.c
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-19 08:23:11
#      History:
=============================================================================*/
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>

// private
FILE *_logger_file = NULL;
int _logger_fd = -1;
pthread_mutex_t _logger_mutex = PTHREAD_MUTEX_INITIALIZER;
Sockaddr_in *_logger_svr_info;

int init_logger(Sockaddr_in *svr_info) {
    _logger_file = fopen("log.txt", "a+");
    _logger_fd = fileno(_logger_file);
    _logger_svr_info = svr_info;
    if(_logger_file == 0) return 0;
    return 1;
}

void close_logger() {
    fclose(_logger_file);
}

void logger_log(Sockaddr_in* src, int id, char* str, int len) {
    char *cpy = malloc(sizeof(char) * strlen(str));
    strcpy(cpy, str);
    struct tm *timeinfo;
    time_t rawtime;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    //time_str = asctime(timeinfo);

    /*
    logger_arg_t *arg = malloc(sizeof(logger_arg_t));
    arg->timeinfo = timeinfo;
    arg->src = src;
    arg->str = str;
    arg->len = len;
    arg->id = id;

    pthread_t thread;
    if((pthread_create(&thread, NULL, logger_handler, (void*)arg)) < 0) {
        perror("ERROR creating thread");
    }
    */

    char* ip = "0.0.0.0";
    int port = ntohs(_logger_svr_info->sin_port);
    if(src) {
        ip = inet_ntoa(src->sin_addr);
        port = ntohs(src->sin_port);
    }

    char to_write[42+len];
    bzero(to_write, 42+len);

    sprintf(to_write, "[%02d/%02d/%02d %03d:%02d:%02d] %03d@%-15s:%05d %s\n",
            timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year - 100,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
            id, ip, port, cpy);

    fwrite(&to_write, strlen(to_write), 1, _logger_file);
    fflush(_logger_file);

    fprintf(stderr, "%s", to_write);
    fflush(stderr);
}

/*
void *logger_handler(void* logger_arg) {
    pthread_detach(pthread_self());

    if(_logger_fd == -1) {
        fprintf(stderr, "[LOGGER] logger was not initialized\n");
    }

    pthread_mutex_lock(&_logger_mutex);

    logger_arg_t *arg = (logger_arg_t*) logger_arg;
    Sockaddr_in *src = arg->src;
    char* ip = "0.0.0.0";
    char *time_str = arg->time_str;
    time_str[strlen(time_str) - 1] = '\0';

    if(src)
        ip = inet_ntoa(src->sin_addr);

    fprintf(stderr, "[LOGGER] Writing log..\n");

    fprintf(_logger_file, "[%s] %s:%d ", time_str, ip, ntohs(src->sin_port));
    for(int i = 0; i < arg->len; i++)
        fprintf(_logger_file, "%c", arg->str[i]);
    fprintf(_logger_file, "\n");

    fflush(_logger_file);

    free(arg);
    pthread_mutex_unlock(&_logger_mutex);

    fprintf(stderr, "[LOGGER] Logger exiting\n");

    return 0;
}
*/
