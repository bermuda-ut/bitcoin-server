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

// maximum number of concurrent threads a client can have
#define CLIENT_THREAD_COUNT 20

// number of threads used to calculate work for the client
#define WORKER_COUNT 10

#include "handler_helper.h"

typedef struct {
    int* newsockfd;
    int* command_len;
    char* command_str;
    int client_id;
} worker_arg_t;

typedef struct {
    void (*worker_func)(worker_arg_t*);
    char* flag;
    worker_arg_t *worker_arg;
} wrapper_arg_t;

typedef struct {
    uint64_t* n;
    uint64_t* answer;
    BYTE* target;
    BYTE* seed;
    pthread_mutex_t *mutex;
} work_btch_arg_t;

typedef struct queue {
    worker_arg_t* worker_arg;
    struct queue *next;
} queue_t;

typedef struct {
    queue_t** work_queue;
    pthread_mutex_t *queue_mutex;
} work_man_arg_t;

extern void *client_handler(void *);

void *handler_wrapper(void *);

void ping_handler(worker_arg_t *);
void pong_handler(worker_arg_t *);
void okay_handler(worker_arg_t *);
void erro_handler(worker_arg_t *);
void soln_handler(worker_arg_t *);
void unkn_handler(worker_arg_t *);
void slep_handler(worker_arg_t *);

void *work_manager(void*);
void *work_btch(void*);


worker_arg_t* pop_queue(queue_t**);
int check_working(char* working);
void add_queue(worker_arg_t*, queue_t**);
void flag_working(char* working);
void unflag_working(char* working);
void free_work_man_arg(work_man_arg_t *arg);

void free_worker_arg(worker_arg_t *arg);
#endif
