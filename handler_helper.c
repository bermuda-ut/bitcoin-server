/*=============================================================================
#     FileName: handler_helper.c
#         Desc: functions that assist handlers in general
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-24 17:21:42
=============================================================================*/
#include "handler_helper.h"
#include "sha256.h"
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// I am sorry.
BYTE BYTE_TWO[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02
};

//==============================================================================

// aka pop head from queue
void rm_tid(queue_t **queue, pthread_mutex_t *mutex) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_mutex_lock(mutex);

    if(*queue != NULL) {
        queue_t* tmp = *queue;
        *queue = (*queue)->next;
        free(tmp);
    }

    pthread_mutex_unlock(mutex);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
}

// aka push tail to queue
void push_tid(queue_t **queue, pthread_mutex_t *mutex, int tid) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_mutex_lock(mutex);

    queue_t *curr = *queue;
    queue_t *new = malloc(sizeof(queue_t));

    new->next = NULL;
    new->thread_id = tid;

    if(curr != NULL) {
        while(curr->next != NULL)
            curr = curr->next;
        curr->next = new;
    } else {
        *queue = new;
    }

    pthread_mutex_unlock(mutex);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
}

// aka read head
int get_tid(queue_t **queue, pthread_mutex_t *mutex) {
    int val = -1;

    (void)*mutex; // lol

    //pthread_mutex_lock(mutex);
    if(*queue != NULL)
        val = (*queue)->thread_id;
    //pthread_mutex_unlock(mutex);

    return val;
}

// aka print plz
void print_queue(queue_t *q) {
    queue_t *curr = q;
    fprintf(stderr, "[ ");
    while(curr) {
        fprintf(stderr, "%d ", curr->thread_id);
        curr = curr->next;
    }
    fprintf(stderr, "]\n");
}

//==============================================================================

int has_command(char** full_cmd_str, int *used_len) {
    for(int i = 0; i < *used_len - 1; i++) {
        if((*full_cmd_str)[i] == '\r' && (*full_cmd_str)[i+1] == '\n') {
            return 1;
        }
    }

    return 0;
}
/**
 * try to get command from existing string
 * command = something that ends with \r\n
 */
char* get_command(char** full_cmd_str, int len, int *used_len) {
    for(int i = 0; i < *used_len - 1; i++) {
        if((*full_cmd_str)[i] == '\r' && (*full_cmd_str)[i+1] == '\n') {
            // 0 1 2 3 4  5  6 7 8 9
            // P I N G \r \n P I N G
            //          ^ i is here
            char *cmd = malloc(sizeof(char) * (i + 1));
            memcpy(cmd, *full_cmd_str, i);

            // null terminate to make our life easier
            cmd[i] = '\0'; 

            char *rest = malloc(sizeof(char) * len);
            memcpy(rest, (*full_cmd_str) + i + 2, len - i - 2);

            free(*full_cmd_str);
            *full_cmd_str = rest;
            *used_len -= (i + 2);

            return cmd;
        }
    }

    return NULL;
}

/**
 * join the commands into the buffer
 */
void join_client_command(char **str, char *command_str, int *str_len, int* used_len) {
    int cmd_len = strlen(command_str);

    while(*str_len < *used_len + cmd_len + 1) {
        *str_len = *str_len * 2;
        *str = realloc(*str, sizeof(char) * *str_len);
    }

    // cnt strcpy >:( stuff are not null terminated.
    memcpy((*str)+*used_len, command_str, cmd_len);
    *used_len += cmd_len;
}

//==============================================================================

/**
 * send formatted message to the client, according to specs
 */
void send_formatted(int *newsockfd, char* info, char* msg) {
    char *to_send = malloc(sizeof(char) * 45);

    for(int i = 0; i < 45; i++)
        to_send[i] = 32;

    memcpy(to_send, info, 4);
    to_send[4] = '\t';

    if(msg) {
        int len = strlen(msg);
        int i = len;

        if(i > 40)
            i = 40;

        memcpy(to_send+5, msg, i);
    }

    to_send[43] = '\r';
    to_send[44] = '\n';

    send_message(newsockfd, to_send, 45);
}

/**
 * send raw message to the client
 */
void send_message(int *newsockfd, char* to_send, int len) {
    int n;

    char to_log[len];
    memcpy(to_log, to_send, len);
    to_log[len-2] = '\0';
    logger_log(NULL, *newsockfd, to_log, len-2);

    if ((n = write(*newsockfd, to_send, len)) <= 0) {
#if DEBUG
        perror("[ SENDMSG ] ERROR writing to socket");
#endif
    }
}

/**
 * print bytes to stream
 */
void byte_print(FILE *stream, BYTE *byte, size_t size) {
    fprintf (stream, "0x");

    for (size_t i = 0; i < size; i++) {
        fprintf(stream, "%02x", byte[i]); 
    }

    fprintf (stream, "\n");
}

/**
 * hex string to byte array
 */
BYTE *hstob(char *hex_string, size_t size) {
    BYTE *val = malloc(sizeof(BYTE) * size);
    bzero(val, size);

    char* pos = hex_string;

    for(size_t count = 0; count < size; count++) {
        sscanf(pos, "%2hhx", val+count);
        pos += 2;
    }

    return val;
}

//==============================================================================
//
BYTE *get_target(uint32_t difficulty) {

    int *alpha_int = malloc(sizeof(int));
    *alpha_int = (difficulty) & 0xFF;
    /*
#if DEBUG
    fprintf(stderr, "[ TARGET ] Calculating target from %x %u\n", difficulty, difficulty);
    fprintf(stderr, "[ TARGET ] Alpha = %d, Alpha - 3 = %d (%x)\n", *alpha_int, *alpha_int - 3, *alpha_int - 3);
#endif
    */

    // init beta
    BYTE beta[32];
    uint256_init(beta);

    //...disgusting :(
    beta[29] = (difficulty >> (8*1)) & 0xFF;
    beta[30] = (difficulty >> (8*2)) & 0xFF;
    beta[31] = (difficulty >> (8*3)) & 0xFF;

    /*
#if DEBUG
    fprintf(stderr, "[ TARGET ] Beta is:   ");
    byte_print(stderr, beta, 32);
    fprintf(stderr, "\n");
#endif
    */

    // init target and exponent for calculation
    BYTE *target = malloc(sizeof(BYTE) * 32);
    BYTE* two_exp = malloc(sizeof(BYTE) * 32);
    uint256_init(target);
    uint256_init(two_exp);

    uint256_exp(two_exp, BYTE_TWO, 8 * (*alpha_int - 3));
    uint256_mul(target, beta, two_exp);

    // whee im so free
    free(alpha_int);
    free(two_exp);

    /*
#if DEBUG
    fprintf(stderr, "[ TARGET ] Target is: ");
    byte_print(stderr, target, 32);
    fprintf(stderr, "\n");
#endif
    */
    return target;
}

BYTE *seed_from_raw(char* raw_seed) {
    BYTE *seed = hstob(raw_seed, 32);
    /*
#if DEBUG
    fprintf(stderr, "[ THREAD ] Parsing seed: ");
    byte_print(stderr, seed, 32);
    fprintf(stderr, "\n");
#endif
    */
    return seed;
}

BYTE *get_x(BYTE* seed, uint64_t solution) {
    /*
#if DEBUG
    fprintf(stderr, "[ GET_X ] Parsing x with solution %lu %lx\n", solution, solution);
#endif
    */

    BYTE *x = malloc(sizeof(BYTE) * 40);
    memcpy(x, seed, 32);

    if(ntohl(solution) != solution) {
        // little endian
        for(int i = 0; i < 8; i++) {
            x[39-i] = (solution >> (8 * i)) & 0xFF;
        }
    } else {
        // big endian
        memcpy(x+32, &solution, 8);
    }
    
    /*
#if DEBUG
    fprintf(stderr, "[ GET_X ] x: ");
    byte_print(stderr, x, 40);
    fprintf(stderr, "\n");
#endif
    */

    return x;
}

int is_valid_soln(BYTE *target, BYTE* seed, uint64_t solution) {
    /*
#if DEBUG
    fprintf(stderr, "[ CHCKSOLN ] Checking..\n");
#endif
    */

    BYTE *x = get_x(seed, solution);
	BYTE buf[SHA256_BLOCK_SIZE];

	SHA256_CTX ctx;

	sha256_init(&ctx);
	sha256_update(&ctx, x, 40);
	sha256_final(&ctx, buf);

	sha256_init(&ctx);
	sha256_update(&ctx, buf, SHA256_BLOCK_SIZE);
	sha256_final(&ctx, buf);

    free(x);

    /*
#if DEBUG
    fprintf(stderr, "[ CHCKSOLN ] y: ");
    byte_print(stderr, buf, 32);
    fprintf(stderr, "\n");
#endif
    */

    return sha256_compare(buf, target);
}

