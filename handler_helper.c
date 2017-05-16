#include "handler_helper.h"
#include "crypto/sha256.h"
#include <string.h>
#include <unistd.h>

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


// hex string to byte
BYTE *hstob(char *hex_string, size_t size) {
    BYTE *val = malloc(sizeof(BYTE) * size);
    bzero(val, size);

    char* pos = hex_string;

    for(size_t count = 0; count < size; count++) {
        sscanf(pos, "%2hhx", &val[count]);
        pos += 2;
    }

    return val;
}

char* get_command(char** full_cmd_str, int len, int* cmd_len) {
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
            *cmd_len = i+1;

            return cmd;
        }
    }

    return NULL;
}

void join_client_command(char **str, char *command_str, int *str_len) {
    int currlen = strlen(*str);

    if(currlen > *str_len - 3) {
        int tmp = *str_len;
        *str_len = *str_len * 2;
        str = realloc(str, sizeof(char) * *str_len);
        bzero(str+tmp, *str_len - tmp);
    }

    strcat(*str, command_str);
}

void send_formatted(int *newsockfd, char* info, char* msg) {
    char *to_send = malloc(sizeof(char) * 45);

    memcpy(to_send, info, 4);
    to_send[4] = '\t';

    for(int i = 0; i < 40; i++) {
        to_send[5+i] = 32;
    }

    if(msg) {
        int len = strlen(msg);
        int i = len;

        if(i > 40)
            i = 40;

        memcpy(to_send+5, msg, i);
    }

    to_send[43] = '\r';
    to_send[44] = '\n';

    send_message(newsockfd, to_send);
}

void send_message(int *newsockfd, char* to_send) {
    int n;

    if ((n = write(*newsockfd, to_send, strlen(to_send))) <= 0) {
        perror("ERROR writing to socket");
    }
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
    beta[29] = (difficulty >> (8*1)) & 0xFF;
    beta[30] = (difficulty >> (8*2)) & 0xFF;
    beta[31] = (difficulty >> (8*3)) & 0xFF;

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
    uint64_t tmp = ntohl(solution);

    memcpy(x, seed, 32);
    if(tmp != solution) {
        // little endian
        BYTE *temp2 = malloc(sizeof(BYTE) * 8);
        memcpy(temp2, &solution, 8);
        for(int i = 0; i < 8; i++) {
            x[32+i] = temp2[7-i];
        }
        free(temp2);

    } else {
        memcpy(x+32, &solution, 8);
    }
    
    fprintf(stderr, "[THREAD] x: ");
    byte_print(stderr, x, 40);

    return x;
}

int is_valid_soln(BYTE *target, BYTE* seed, uint64_t solution) {
    fprintf(stderr, "[THREAD] checking if cat is valid\n");

    BYTE *x = get_x(seed, solution);
	BYTE buf[SHA256_BLOCK_SIZE];

	SHA256_CTX ctx;

	sha256_init(&ctx);
	sha256_update(&ctx, x, 40);
	sha256_final(&ctx, buf);

	sha256_init(&ctx);
	sha256_update(&ctx, buf, SHA256_BLOCK_SIZE);
	sha256_final(&ctx, buf);

    fprintf(stderr, "[THREAD] y: ");
    byte_print(stderr, buf, 32);

    fprintf(stderr, "[THREAD] t: ");
    byte_print(stderr, target, 32);

    return sha256_compare(buf, target);
}

