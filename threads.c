/*=============================================================================
#     FileName: threads.c
#         Desc:  
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-15 19:16:52
#      History:
=============================================================================*/
#include "threads.h"

char *init_avail_flags(int count) {
    char* flags = malloc(sizeof(char) * count);
    bzero(flags, count);
    return flags;
}

int get_avail_thread(char* flags, int count) {
    for(int i = 0; i < count; i++) {
        if(flags[i] == 0) {
            flags[i] = 1;
            return i;
        }
    }
    return -1;
}

void reset_flag(char* flag) {
    *flag = 0;
}
