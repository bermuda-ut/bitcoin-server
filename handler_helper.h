/*=============================================================================
#     FileName: handler_helper.h
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-16 13:28:11
#      History:
=============================================================================*/
#ifndef HANDLER_HELPER
#define HANDLER_HELPER

#include "uint256.h"
#include "logger.h"
#include <arpa/inet.h>

typedef struct queue {
    int thread_id;
    struct queue *next;
} queue_t;

// parsing commands
extern char* get_command(char** full_cmd_str, int len, int *used_len);
extern void join_client_command(char **str, char *command_str, int *str_len, int* used_len);

// for solution stuff
extern int is_valid_soln(BYTE *target, BYTE* seed, uint64_t solution);
extern BYTE *get_x(BYTE* seed, uint64_t solution);
extern BYTE *seed_from_raw(char* raw_seed);
extern BYTE *get_target(uint32_t difficulty);

// for message sending
void send_message(int *newsockfd, char* to_send, int len);
extern void send_formatted(int *newsockfd, char* info, char* msg);

// queue stuff, thread safe
extern int get_tid(queue_t **, pthread_mutex_t*);
extern void push_tid(queue_t **queue, pthread_mutex_t *mutex, int tid);
extern void rm_tid(queue_t **queue, pthread_mutex_t *mutex);
extern void print_queue(queue_t *q);

// etc
extern BYTE *hstob(char *hex_string, size_t size);
extern void byte_print(FILE *stream, BYTE *byte, size_t size);

#endif
