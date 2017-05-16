/*=============================================================================
#     FileName: handler.h
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-15 19:16:55
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
    char* flag;
    int client_id;
} worker_arg_t;

extern void *client_handler(void *);

void *ping_handler(void *);
void *pong_handler(void *);
void *okay_handler(void *);
void *erro_handler(void *);
void *soln_handler(void *);
void *unkn_handler(void *);
void *work_handler(void *);
void *slep_handler(void *);

int is_valid_soln(BYTE *, BYTE *, uint64_t);

void send_message(int *, char*);

void free_worker_arg(worker_arg_t *arg);

#endif
