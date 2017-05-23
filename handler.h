/*=============================================================================
#     FileName: handler.h
#         Desc: handler definitions
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-23 04:53:31
=============================================================================*/
#ifndef HANDLER
#define HANDLER

#include "handler_helper.h"
#include <semaphore.h>

// Simple starting point for a command length
#define INIT_CMD_STR_SIZE 128

// maximum number of concurrent threads a client can have
// Using the answer to the universe >:)
// In real servers, you wouldn't have such high values
#define CLIENT_THREAD_COUNT 42

// maximum number of concurrent work processing for each client
#define CONCURRENT_WORK_COUNT 5

typedef struct {
    int *newsockfd;
    char *command_str;
    int thread_id;
    int client_id;

    // strictly for work and abrt threads only
    queue_t **work_queue;
    pthread_mutex_t *queue_mutex;
    pthread_mutex_t *worker_mutex;
    pthread_t *thread_pool;
    char *pool_flag;
    sem_t *worker_sem;
} worker_arg_t;

typedef struct {
    void (*worker_func)(worker_arg_t*);
    char *flag;
    worker_arg_t *worker_arg;
} wrapper_arg_t;

typedef struct {
    uint64_t *solution;
    //uint64_t* n;
    uint64_t start;
    uint64_t end;
    int *cancelled;
    int btch_id;
    BYTE *target;
    BYTE *seed;
    pthread_mutex_t* sol_mutex;
} btch_arg_t;

typedef struct {
    pthread_t **btches;
    pthread_t *thread_pool;
    char *pool_flag;
    int thread_count;
    int thread_id;
    int *cancelled;

    queue_t **tid_queue;
    pthread_mutex_t *queue_mutex;
    sem_t *worker_sem;
} cleanup_arg_t;

extern void *client_handler(void *);

void *work_btch(void *);
void *handler_wrapper(void *);
void work_handler_cleanup(void*);

void work_handler(worker_arg_t *);
void abrt_handler(worker_arg_t *);
void ping_handler(worker_arg_t *);
void pong_handler(worker_arg_t *);
void okay_handler(worker_arg_t *);
void erro_handler(worker_arg_t *);
void soln_handler(worker_arg_t *);
void unkn_handler(worker_arg_t *);
void slep_handler(worker_arg_t *);

#endif

