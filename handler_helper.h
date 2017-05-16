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
#include <arpa/inet.h>

extern BYTE *hstob(char *hex_string, size_t size);
extern char* get_command(char** full_cmd_str);
extern void join_client_command(char **str, char *command_str, int *str_len);

extern int is_valid_soln(BYTE *target, BYTE* seed, uint64_t solution);
extern BYTE *get_x(BYTE* seed, uint64_t solution);
extern BYTE *seed_from_raw(char* raw_seed);
extern BYTE *get_target(uint32_t difficulty);

extern void send_message(int *newsockfd, char* to_send);
extern void byte_print(FILE *stream, BYTE *byte, size_t size);
#endif
