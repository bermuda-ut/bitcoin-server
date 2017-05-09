#include <pthread.h>
#include <semaphore.h>
#include "driver.h"

typedef struct {
    int* newsockfd;
    char* flags;
    int i;
} thread_arg_t;

char *init_avail_flags(int len);
int get_avail_thread(char* flags, int len);
