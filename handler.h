/*=============================================================================
#     FileName: handler.h
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-09 14:33:27
#      History:
=============================================================================*/
#define INIT_CMD_STR_SIZE 256

extern void *client_handler(void *thread_arg);


void ping_handler(int *newsockfd, char *command_str);
void pong_handler(int *newsockfd, char *command_str);
void okay_handler(int *newsockfd, char *command_str);
void erro_handler(int *newsockfd, char *command_str);

void send_message(int *newsockfd, char* to_send);

void join_client_command(char **dest, char* src, int *destlen);
char* get_command(char** full_cmd_str);
