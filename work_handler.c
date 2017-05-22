/*=============================================================================
#     FileName: work_handler.c
#         Desc: work handler definition
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-22 21:49:11
=============================================================================*/
#include "handler.h"
#include "threads.h"
#include "driver.h"
#include "logger.h"

void work_handler_cleanup(void* cleanup_arg) {
    cleanup_arg_t *arg = (cleanup_arg_t*) cleanup_arg;
    //fprintf(stderr, "[THREAD] Cleanup: %d btches remain\n", arg->thread_count);

    *(arg->cancelled) = 1;

    queue_t **tid_queue = arg->tid_queue;
    pthread_mutex_t *mutex = arg->queue_mutex;
    int thread_id = arg->thread_id;

    //fprintf(stderr, "[THREAD] tid cleanup, thread is %d\n", thread_id);
    while(get_tid(tid_queue, mutex) != thread_id) {
        //fprintf(stderr, "Waiting for current pid \n");
        sleep(1);
    }

    rm_tid(tid_queue, mutex);

    //fprintf(stderr, "[THREAD] Finish cleanup thread %d\n", thread_id);
}

void work_handler(worker_arg_t *arg) {
    //fprintf(stderr, "[WORKER] worker spawned\n");
    int cancelled = 0;
    int thread_id = arg->thread_id;
    int *newsockfd = arg->newsockfd;
    char *command_str = arg->command_str;
    queue_t **tid_queue = arg->work_queue;
    pthread_mutex_t *queue_mutex = arg->queue_mutex;
    sem_t *worker_sem = arg->worker_sem;
    cleanup_arg_t cleanup_arg;

    cleanup_arg.tid_queue = tid_queue;
    cleanup_arg.thread_id = thread_id;
    cleanup_arg.queue_mutex = queue_mutex;
    cleanup_arg.cancelled = &cancelled;
    cleanup_arg.btches = NULL;
    cleanup_arg.thread_count = 0;
    cleanup_arg.worker_sem = worker_sem;
    pthread_cleanup_push(work_handler_cleanup, (void*)&cleanup_arg);

    uint32_t difficulty;
    uint64_t n;
    char raw_seed[64];
    int thread_count;

    sscanf(command_str + 5, "%x %s %lx %x", &difficulty, raw_seed, &n, &thread_count);
    difficulty = ntohl(difficulty);
    //n = ntohl(n);
    /*
    if(thread_count > WORKER_COUNT_MAX)
        thread_count = WORKER_COUNT_MAX;
    */

    uint64_t solution = 0;
    BYTE *target = get_target(difficulty);
    BYTE *seed = seed_from_raw(raw_seed);
    pthread_mutex_t sol_mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_t *btches = malloc(sizeof(pthread_t) * thread_count);
    btch_arg_t btch_args[thread_count];
    cleanup_arg.btches = &btches;
    cleanup_arg.thread_count = thread_count;

    //fprintf(stderr, "[THREAD] Worker %d waiting for %s\n", thread_id, command_str);
    sem_wait(worker_sem);
    //fprintf(stderr, "[THREAD] Worker %d is now processing %s\n", thread_id, command_str);

    uint64_t chunk = (UINT64_MAX - n) / thread_count;

    //fprintf(stderr, "n is %lu %lx\n", n, n);
    //fprintf(stderr, "chunk size is %lu\n", chunk);
    for(int i = 0; i < thread_count; i++) {
        btch_args[i].solution = &solution;
        btch_args[i].target = target;
        btch_args[i].seed = seed;
        btch_args[i].cancelled = &cancelled;
        btch_args[i].sol_mutex = &sol_mutex;
        btch_args[i].btch_id = i;
        btch_args[i].start = n + i * chunk;

        if(i == thread_count - 1)
            btch_args[i].end = UINT64_MAX;
        else
            btch_args[i].end = n + (i + 1) * chunk;

        if((pthread_create(btches + i, NULL, work_btch, (void*)(btch_args + i))) < 0) {
            perror("ERROR creating thread");
        }
        //fprintf(stdout, "[THREAD] Workerbtch made %lu\n", btches[i]);
    }

    while(solution == 0) {
        //fprintf(stdout, "[THREAD] Waiting for solution..\n");
        sleep(1);
    }

    int curr;
    while((curr = get_tid(tid_queue, queue_mutex)) != thread_id) {
        fprintf(stderr, "[THREAD] Worker %d is waiting for its turn, current is %d\n", thread_id, curr);
        sleep(1);
    }
    sem_post(worker_sem);

    pthread_mutex_lock(arg->worker_mutex);
    //fprintf(stdout, "[THREAD] Solution found!\n");
    char* result = malloc(sizeof(char) * 98);
    char print_seed[65];
    bzero(print_seed, 65);
    memcpy(print_seed, raw_seed, 64);
    difficulty = htonl(difficulty);
    sprintf(result, "SOLN %08x %s %016lx\r\n", difficulty, print_seed, solution);
    //fprintf(stdout, "[THREAD] Sending!\n");
    send_message(newsockfd, result, 97);
    pthread_mutex_unlock(arg->worker_mutex);

    //fprintf(stderr, "[THREAD] Worker %d cleaning up\n", thread_id);
    pthread_cleanup_pop(1);
    //fprintf(stderr, "[THREAD] Worker %d exiting\n", thread_id);
    // do shit
}

void *work_btch(void *btch_arg) {
    btch_arg_t *arg = (btch_arg_t*) btch_arg;
    BYTE* target = arg->target;
    BYTE* seed = arg->seed;
    uint64_t start = arg->start;
    uint64_t end = arg->end;
    uint64_t* solution = arg->solution;
    pthread_mutex_t* mutex = arg->sol_mutex;
    int *cancelled = arg->cancelled;

    uint64_t trying = start;
    //fprintf(stderr, "thread trying from %lu to %lu (inclusive)\n", start, end);
    while(1) {
        //fprintf(stderr, "trying %lu\n",trying);
        int res;

        if(*cancelled || trying >= end)
            break;

        if((res = is_valid_soln(target, seed, trying)) == -1) {
            //fprintf(stderr, "[WORKBTCH] Solution found %lu\n", trying);
            pthread_mutex_lock(mutex);
            *solution = trying;
            *cancelled = 1;
            break;
        //} else {
            //fprintf(stderr, "[WORKBTCH] Attempted.. Retrying in 1 second\n");
        }

        trying += 1;
    }

    //fprintf(stderr, "[WORKBTCH] Work bitch %d is now dying\n", arg->btch_id);
    return 0;
}


