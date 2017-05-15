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

#define WORKER_COUNT 10

#define INIT_CMD_STR_SIZE 128
#include "uint256.h"
#include <arpa/inet.h>

typedef struct {
    int* newsockfd;
    char* command_str;
    int client_id;
} worker_arg_t;

extern void *client_handler(void *);

void *ping_handler(void *);
void *pong_handler(void *);
void *okay_handler(void *);
void *erro_handler(void *);
void *soln_handler(void *);
void *unkn_handler(void *);

int is_valid_soln(BYTE *, BYTE *, uint64_t);

void send_message(int *, char*);

static void free_worker_arg(worker_arg_t *arg) {
    free(arg);
}

// hex string to byte
static BYTE *hstob(char *hex_string, size_t size) {
    BYTE *val = malloc(sizeof(BYTE) * size);
    bzero(val, size);

    char* pos = hex_string;

    for(size_t count = 0; count < size; count++) {
        sscanf(pos, "%2hhx", &val[count]);
        pos += 2;
    }

    return val;
}

//TOFIX?
static BYTE *itob(int i, size_t size) {
    BYTE *val = malloc(sizeof(BYTE) * size);
    bzero(val, size);

    for(size_t count = 0; count < sizeof(i); count++) {
        val[size - 1 - count] = (i >> (sizeof(i) * count)) & 0xff;
    }

    return val;
}

/*
void join_client_command(char **dest, char* src, int *destlen);
char* get_command(char** full_cmd_str);
*/

static char* get_command(char** full_cmd_str) {
    int len = strlen(*full_cmd_str);

    for(int i = 0; i < len - 1; i++) {
        if((*full_cmd_str)[i] == '\r' && (*full_cmd_str)[i+1] == '\n') {
            // 0 1 2 3 4  5  6 7 8 9
            // P I N G \r \n P I N G
            //          ^ i is here

            char *cmd = malloc(sizeof(char) * (i + 1));

            for(int j = 0; j < i; j++) {
                cmd[j] = (*full_cmd_str)[j];
            }
            cmd[i] = '\0';

            char *rest = malloc(sizeof(char) * len);
            strcpy(rest, (*full_cmd_str) + i + 2);

            free(*full_cmd_str);
            *full_cmd_str = rest;

            return cmd;
        }
    }

    return NULL;
}

static void join_client_command(char **str, char *command_str, int *str_len) {
    int currlen = strlen(*str);

    if(currlen > *str_len - 3) {
        int tmp = str_len;
        *str_len = *str_len * 2;
        str = realloc(str, sizeof(char) * *str_len);
        bzero(str+tmp, str_len - tmp);
    }

    strcat(*str, command_str);
}

#endif
