#include "threads.h"

extern char *init_avail_flags(int count) {
    char* flags = malloc(sizeof(char) * count);
    bzero(flags, CLIENT_COUNT);
    return flags;
}

extern int get_avail_thread(char* flags, int count) {
    int i = 0;
    while(i < count) {
        if(flags[i] == 0) {
            flags[i] = 1;
            return i;
        }
    }
    return -1;
}
