/*=============================================================================
#     FileName: handler.h
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-16 16:58:17
#      History:
=============================================================================*/
#ifndef HANDLER
#define HANDLER

// simple starting point for a command length
#define INIT_CMD_STR_SIZE 128

// maximum number of concurrent command a client can have it processed
// does not include the worker thread
#define CLIENT_THREAD_COUNT 20

// number of threads used to calculate work for the client
#define WORKER_COUNT 10

#include "handler_helper.h"

typedef struct {
    int* newsockfd;
    int* command_len;
    char* command_str;
    pthread_mutex_t* send_msg_mutex;
    int client_id;
} worker_arg_t;

typedef struct {
    void (*worker_func)(worker_arg_t*);
    char* flag;
    worker_arg_t *worker_arg;
} wrapper_arg_t;

typedef struct work_queue {
    char* cmd;
    struct work_queue *next;
} work_queue_t;

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
