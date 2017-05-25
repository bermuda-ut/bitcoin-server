/*=============================================================================
#     FileName: threads.c
#         Desc: thread basics
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-15 19:16:52
=============================================================================*/
#include "threads.h"

char *init_avail_flags(int count) {
    char* flags = malloc(sizeof(char) * count);
    bzero(flags, count);
    return flags;
}

int get_avail_thread(char* flags, int count, pthread_mutex_t* mutex) {
    int val = -1;

    int oldstate = 0;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
    pthread_mutex_lock(mutex);
    for(int i = 0; i < count; i++) {
        if(flags[i] == 0) {
            flags[i] = 1;
            val = i;
            break;
        }
    }
    pthread_mutex_unlock(mutex);
    pthread_setcancelstate(oldstate, NULL);

    return val;
}

int count_avail_thread(char* flags, int count, pthread_mutex_t* mutex) {
    int t = 0;

    int oldstate = 0;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
    pthread_mutex_lock(mutex);
    for(int i = 0; i < count; i++) {
        if(flags[i] == 0) {
            t++;
        }
    }
    pthread_mutex_unlock(mutex);
    pthread_setcancelstate(oldstate, NULL);

    return t;
}
int check_avail_thread(char* flags, int count, pthread_mutex_t* mutex) {
    int val = -1;

    int oldstate = 0;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
    pthread_mutex_lock(mutex);
    for(int i = 0; i < count; i++) {
        if(flags[i] == 0) {
            val = 1;
            break;
        }
    }
    pthread_mutex_unlock(mutex);
    pthread_setcancelstate(oldstate, NULL);

    return val;
}

void reset_flag(char* flag, pthread_mutex_t* mutex) {
    int oldstate = 0;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
    pthread_mutex_lock(mutex);
    *flag = 0;
    pthread_mutex_unlock(mutex);
    pthread_setcancelstate(oldstate, NULL);
}
