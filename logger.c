/*=============================================================================
#     FileName: logger.c
#         Desc: logging to a file :)
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-24 17:24:56
=============================================================================*/
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>

// Private.. in a dirty method. Should be in .h but wtv
FILE *_logger_file = NULL;
int _logger_fd = -1;
pthread_mutex_t _logger_mutex = PTHREAD_MUTEX_INITIALIZER;
Sockaddr_in *_logger_svr_info;

const char* _coin_ascii = "                     ______________\n\
        __,.,---'''''              '''''---..._\n\
     ,-'             ,:::::,,:::::,           '`-.\n\
    |             ''::     ::     ::''           |\n\
    |            '':::.....::....,:::''           |\n\
    |'-.._        ````````````````````       __,,-|\n\
     '-.._''`---.....______________.....---''__,,-\n\
          ''`---.....______________.....---''";

const char* _svr_ascii =
"  ___ _ _    ___     _        ___\n\
 | _ |_) |_ / __|___(_)_ _   / __| ___ _ ___ _____ _ _ \n\
 | _ \\ |  _| (__/ _ \\ | ' \\  \\__ \\/ -_) '_\\ V / -_) '_|\n\
 |___/_|\\__|\\___\\___/_|_||_| |___/\\___|_|  \\_/\\___|_|";

/*
 * Initialize logger
 * */
int init_logger(Sockaddr_in *svr_info) {
    _logger_file = fopen("log.txt", "w");
    _logger_fd = fileno(_logger_file);
    _logger_svr_info = svr_info;

    // something went wrong
    if(_logger_file == 0) return 0;

    char* mode = "Submission";
    char* mode2 = "Logless STDOUT";
#if DEBUG
    mode = "Debug";
#endif
#if LOG_OUTPUT
    mode2 = "Log STDOUT";
#endif

    fprintf(stdout, "\n\n%s\n\n", _coin_ascii);
    fprintf(stdout, "%s\n\n", _svr_ascii);
    fprintf(stdout, "--------------------------------------------------------\n\
 Author     : Max Lee, max@mirrorstairstudio.com\n\
 Server Mode: %s, %s\n\
 Date       : 24/MAY/17\n\n\
 Multithreaded Bitcoin Server based on CS Project 2\n\
 Written in blood and tears, not from this project </3\n\
 \n\
 mirrorstairstudio.com                  mallocsizeof.me\n\
--------------------------------------------------------\n", mode, mode2);
    fflush(stdout);

    return 1;
}

/*
 * close logger
 * */
void close_logger() {
    fclose(_logger_file);
}

/*
 * log into the file, thread safe
 * */
void logger_log(Sockaddr_in* src, int id, char* str, int len) {
    pthread_mutex_lock(&_logger_mutex);
    char *cpy = malloc(sizeof(char) * strlen(str));
    struct tm *timeinfo = malloc(sizeof(*timeinfo));
    time_t rawtime;
    time(&rawtime);
    char to_write[42+len];
    char* ip = "0.0.0.0";
    char* to = "<";
#if LOG_OUTPUT
    char svr = 1;
#endif

    bzero(to_write, 42+len);

    strcpy(cpy, str);
    localtime_r(&rawtime, timeinfo);

    int port = ntohs(_logger_svr_info->sin_port);
    if(src) {
#if LOG_OUTPUT
        svr = 0;
#endif
        to = "@";
        ip = inet_ntoa(src->sin_addr);
        port = ntohs(src->sin_port);
    }

    sprintf(to_write, "[%02d/%02d/%02d %02d:%02d:%02d] %03d %s %-15s:%05d %s\n",
            timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year - 100,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
            id, to, ip, port, cpy);

#if LOG_OUTPUT
    if((LOG_ONLY_CLIENT && !svr) || !LOG_ONLY_CLIENT) {
        fprintf(stdout, "%s", to_write);
        fflush(stdout);
    }
#endif

    fwrite(&to_write, strlen(to_write), 1, _logger_file);
    fflush(_logger_file);

    free(timeinfo);
    free(cpy);
    pthread_mutex_unlock(&_logger_mutex);
}

