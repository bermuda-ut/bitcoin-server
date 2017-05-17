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

typedef struct queue {
    int thread_id;
    struct queue* next;
} queue_t;

typedef struct {
    int* newsockfd;
    char* command_str;
    int thread_id;
    int client_id;

    // strictly for work and abrt threads only
    queue_t **work_queue;
    pthread_mutex_t *queue_mutex;
    pthread_t *thread_pool;
    char* pool_flag;
} worker_arg_t;

typedef struct {
    void (*worker_func)(worker_arg_t*);
    char* flag;
    worker_arg_t *worker_arg;
} wrapper_arg_t;

typedef struct {
    uint64_t* solution;
    uint64_t* n;
    BYTE* target;
    BYTE* seed;
    pthread_mutex_t* sol_mutex;
} btch_arg_t;

extern void *client_handler(void *);

void *work_btch(void *);
void *handler_wrapper(void *);
void work_handler_cleanup(void* ptr_btches);

void work_handler(worker_arg_t *);
void abrt_handler(worker_arg_t *);
void ping_handler(worker_arg_t *);
void pong_handler(worker_arg_t *);
void okay_handler(worker_arg_t *);
void erro_handler(worker_arg_t *);
void soln_handler(worker_arg_t *);
void unkn_handler(worker_arg_t *);
void slep_handler(worker_arg_t *);

void free_worker_arg(worker_arg_t *arg);

int get_tid(queue_t *, pthread_mutex_t*);
void push_tid(queue_t **queue, pthread_mutex_t *mutex, int tid);
void rm_tid(queue_t **queue, pthread_mutex_t *mutex);

#endif
