/*=============================================================================
#     FileName: handler.h
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-16 13:28:11
#      History:
=============================================================================*/
#ifndef HANDLER
#define HANDLER

#define INIT_CMD_STR_SIZE 128
#define CLIENT_THREAD_COUNT 20
#define WORKER_COUNT 10

#include "handler_helper.h"

typedef struct {
    int* newsockfd;
    char* command_str;
    int client_id;
} worker_arg_t;

typedef struct {
    void (*worker_func)(worker_arg_t*);
    char* flag;
    worker_arg_t *worker_arg;
} wrapper_arg_t;

extern void *client_handler(void *);

void *handler_wrapper(void *);

void ping_handler(worker_arg_t *);
void pong_handler(worker_arg_t *);
void okay_handler(worker_arg_t *);
void erro_handler(worker_arg_t *);
void soln_handler(worker_arg_t *);
void unkn_handler(worker_arg_t *);
void work_handler(worker_arg_t *);
void slep_handler(worker_arg_t *);

void free_worker_arg(worker_arg_t *arg);

#endif
