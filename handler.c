/*=============================================================================
#     FileName: handler.c
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-14 21:46:54
=============================================================================*/
#include "handler.h"
#include "threads.h"
#include "driver.h"
#include "crypto/sha256.h"

static BYTE BYTE_TWO[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02
};

void *client_handler(void *thread_arg) {
    thread_arg_t *args = (thread_arg_t*) thread_arg;

	char buffer[BUFFER_LEN];
    char *flags = args->flags;
    int *newsockfd = args->newsockfd,
        i = args->i;
    
    int n,
        *recv_str_len = malloc(sizeof(int));
    *recv_str_len = INIT_CMD_STR_SIZE;

    char *cmd,
        **recieved_string = malloc(sizeof(char*));
    *recieved_string = malloc(sizeof(char) * *recv_str_len);

    fprintf(stderr, "[THREAD] Thread Created for Client %d\n", i);

    while(1) {
        bzero(buffer, BUFFER_LEN);

        fprintf(stderr, "[THREAD] Waiting for Client %d input\n", i);

        if ((n = read(*newsockfd, buffer, BUFFER_LEN-1)) < 0) {
            perror("ERROR reading from socket");
            break;

        } else if(n == 0) {
            // end of file
            fprintf(stderr, "[THREAD] Client %d disconnected\n", i);
            break;
        }

        join_client_command(recieved_string, buffer, recv_str_len);

        fprintf(stderr, "[THREAD] Client %d sent: %s", i, buffer);
        fprintf(stderr, "[THREAD] Client %d cmds: %s\n", i, *recieved_string);

        /*
        fprintf(stderr, "         Breakdown: ");
        for(int i = 0; i < (int)strlen(*recieved_string); i++) {
            fprintf(stderr, "%d ", (*recieved_string)[i]);
        }
        fprintf(stderr, "\n");
        */

        while((cmd = get_command(recieved_string)) != NULL) {
            fprintf(stderr, "Was able to get a command: %s\n", cmd);

            if(strlen(cmd) > 4 && cmd[4] == ' ') {
                cmd[4] = '\0';
            }

            if(strcmp("PING", cmd) == 0) {
                ping_handler(newsockfd, cmd);

            } else if(strcmp("PONG", cmd) == 0) {
                pong_handler(newsockfd, cmd);

            } else if(strcmp("OKAY", cmd) == 0) {
                okay_handler(newsockfd, cmd);

            } else if(strcmp("ERRO", cmd) == 0) {
                erro_handler(newsockfd, cmd);

            } else if(strcmp("SOLN", cmd) == 0) {
                soln_handler(newsockfd, cmd);

            } else {
                unkn_handler(newsockfd, cmd);
            }

            free(cmd);
        }
    }

    flags[i] = 0;
    close(*newsockfd);

    return 0;
}


void byte_print(FILE *stream, BYTE *byte, size_t size) {
    fprintf (stream, "0x");

    for (size_t i = 0; i < size; i++) {
        fprintf(stream, "%02x", byte[i]); 
    }

    fprintf (stream, "\n");
}

BYTE *get_target(uint32_t difficulty) {
    fprintf(stderr, "[THREAD] Calculating target from %x %u\n", difficulty, difficulty);

    int *alpha_int = malloc(sizeof(int));

    *alpha_int = (difficulty) & 0xFF;
    fprintf(stderr, "[THREAD] alpha = %d, alpha - 3 = %d (%x)\n", *alpha_int, *alpha_int - 3, *alpha_int - 3);

    BYTE beta[32];
    uint256_init(beta);

    //...disgusting
    beta[30] = (difficulty >> (8*1)) & 0xFF;
    beta[31] = (difficulty >> (8*2)) & 0xFF;
    beta[32] = (difficulty >> (8*3)) & 0xFF;

    fprintf(stderr, "[THREAD] beta is:   ");
    byte_print(stderr, beta, 32);

    BYTE *target = malloc(sizeof(BYTE) * 32);
    BYTE* two_exp = malloc(sizeof(BYTE) * 32);
    uint256_init(target);
    uint256_init(two_exp);

    uint256_exp(two_exp, BYTE_TWO, 8 * (*alpha_int - 3));
    uint256_mul(target, beta, two_exp);

    free(alpha_int);
    free(two_exp);

    fprintf(stderr, "[THREAD] target is: ");
    byte_print(stderr, target, 32);

    return target;
}

BYTE *seed_from_raw(char* raw_seed) {
    fprintf(stderr, "[THREAD] Parsing seed: ");

    BYTE *seed = hstob(raw_seed, 32);
    byte_print(stderr, seed, 32);

    return seed;
}

BYTE *get_x(BYTE* seed, uint64_t solution) {
    fprintf(stderr, "[THREAD] Parsing x with solution %lu %lx\n", solution, solution);

    BYTE *x = malloc(sizeof(BYTE) * 40);
    memcpy(x, seed, 32);

    /*
    memcpy(x+32, &solution, 8);
    */
    BYTE *temp = malloc(sizeof(BYTE) * 8);
    memcpy(temp, &solution, 8);
    for(int i = 0; i < 8; i++) {
        x[32 + i] = temp[7-i];
    }
    free(temp);

    /*
    BYTE *x2 = malloc(sizeof(BYTE) * 40);
    for(int i = 0; i < 40; i++)
        x2[39-i] = x[i];
    */

    fprintf(stderr, "[THREAD] x: ");
    byte_print(stderr, x, 40);

    return x;
}

int is_valid_soln(BYTE *target, BYTE* seed, uint64_t solution) {
    fprintf(stderr, "[THREAD] checking if cat is valid\n");

    BYTE *x = get_x(seed, solution);
	BYTE y[SHA256_BLOCK_SIZE];

	SHA256_CTX ctx;

	sha256_init(&ctx);
	sha256_update(&ctx, x, 40);
	sha256_final(&ctx, y);

	sha256_init(&ctx);
	sha256_update(&ctx, y, SHA256_BLOCK_SIZE);
	sha256_final(&ctx, y);

    fprintf(stderr, "[THREAD] y: ");
    byte_print(stderr, y, 32);

    fprintf(stderr, "[THREAD] t: ");
    byte_print(stderr, target, 32);

    return sha256_compare(y, target);
}

void soln_handler(int *newsockfd, char *command_str) {
    uint32_t difficulty;
    uint64_t solution;
    char raw_seed[64];
    
    sscanf(command_str + 5, "%x %s %lx\r\n", &difficulty, raw_seed, &solution);
    difficulty = ntohl(difficulty);
    //solution   = ntohl(solution);

    BYTE *target = get_target(difficulty);
    BYTE *seed = seed_from_raw(raw_seed);

    /*
    char *to_send = malloc(sizeof(char)*9999);
    */
    fprintf(stderr, "Calculating solution for d:%u s:%lu\r\n", difficulty, solution);

    fprintf(stderr, "[THREAD] Handling SOLN\n");

    int res;
    if((res = is_valid_soln(target, seed, solution)) == -1) {
        send_message(newsockfd, "OKAY\r\n");
    } else {
        fprintf(stderr, "[THREAD] Solution result %d\n", res);
        send_message(newsockfd, "ERRO\tNot a valid solution\r\n");
    }

    fprintf(stderr, "[THREAD] Handling SOLN Success\n");
}

void unkn_handler(int *newsockfd, char *command_str) {
    char to_send[45] = "ERRO\tUnknown command.\r\n";
    send_message(newsockfd, to_send);
}

void erro_handler(int *newsockfd, char *command_str) {
    char to_send[45] = "ERRO\tThis is used to send YOU errors =.=\r\n";
    fprintf(stderr, "[THREAD] Handling ERRO\n");
    send_message(newsockfd, to_send);
    fprintf(stderr, "[THREAD] Handling ERRO Success\n");
}

void okay_handler(int *newsockfd, char *command_str) {
    char to_send[45] = "ERRO\tDude it's not okay to send OKAY okay?\r\n";
    fprintf(stderr, "[THREAD] Handling OKAY\n");
    send_message(newsockfd, to_send);
    fprintf(stderr, "[THREAD] Handling OKAY Success\n");
}

void pong_handler(int *newsockfd, char *command_str) {
    char to_send[45] = "ERRO\tPONG is strictly reserved for server\r\n";
    fprintf(stderr, "[THREAD] Handling PONG\n");
    send_message(newsockfd, to_send);
    fprintf(stderr, "[THREAD] Handling PONG Success\n");
}

void ping_handler(int *newsockfd, char *command_str) {
    fprintf(stderr, "[THREAD] Handling PING\n");
    send_message(newsockfd, "PONG\r\n");
    fprintf(stderr, "[THREAD] Handling PING Success\n");
}

//TODO: 40 byte all the time
void send_message(int *newsockfd, char* to_send) {
    int n;
    if ((n = write(*newsockfd, to_send, strlen(to_send))) <= 0) {
        perror("ERROR writing to socket");
    }
}

