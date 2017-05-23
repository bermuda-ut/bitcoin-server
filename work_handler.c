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
#if DEBUG
    fprintf(stderr, "[ CLEANUP ] Cleanup: %d btches remain\n", arg->thread_count);
#endif

    // cancel work btches
    *(arg->cancelled) = 1;

    queue_t **tid_queue = arg->tid_queue;
    pthread_mutex_t *mutex = arg->queue_mutex;
    int thread_id = arg->thread_id;

    while(get_tid(tid_queue, mutex) != thread_id) {
        //fprintf(stderr, "Waiting for current pid \n");
        sleep(1);
    }

    rm_tid(tid_queue, mutex);

    reset_flag(arg->pool_flag + thread_id);
#if DEBUG
    fprintf(stderr, "[ CLEANUP ] Finish cleanup thread %d\n", thread_id);
#endif
}

void work_handler(worker_arg_t *arg) {
    int cancelled = 0;
    int thread_id = arg->thread_id;
    int *newsockfd = arg->newsockfd;
    char *command_str = arg->command_str;
    queue_t **tid_queue = arg->work_queue;
    pthread_mutex_t *queue_mutex = arg->queue_mutex;
    sem_t *worker_sem = arg->worker_sem;
    cleanup_arg_t cleanup_arg;

    if(strlen(command_str) != 98) {
#if DEBUG
        fprintf(stderr, "[ WORKMAN ] Invalid work!\n");
#endif
        send_formatted(newsockfd, "ERRO", "Not a valid work");
        return;
    }

    // need to have a cleaner to clean this shit up
    cleanup_arg.tid_queue = tid_queue;
    cleanup_arg.thread_id = thread_id;
    cleanup_arg.pool_flag = arg->pool_flag;
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

    // just like.. kinda like SOLN
    sscanf(command_str + 5, "%x %s %lx %x", &difficulty, raw_seed, &n, &thread_count);
    difficulty = ntohl(difficulty);

    uint64_t solution = 0;
    BYTE *target = get_target(difficulty);
    BYTE *seed = seed_from_raw(raw_seed);
    pthread_mutex_t sol_mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_t *btches = malloc(sizeof(pthread_t) * thread_count);
    btch_arg_t btch_args[thread_count];
    cleanup_arg.btches = &btches;
    cleanup_arg.thread_count = thread_count;

#if DEBUG
    fprintf(stderr, "[ WORKMAN ] Worker %d waiting for %s\n", thread_id, command_str);
#endif

    // only X number of workers can concurrently process their stuff
    sem_wait(worker_sem);
#if DEBUG
    fprintf(stderr, "[ WORKMAN ] Worker %d is now processing %s\n", thread_id, command_str);
#endif

    uint64_t chunk = (UINT64_MAX - n) / thread_count;

    for(int i = 0; i < thread_count; i++) {
        btch_args[i].solution = &solution;
        btch_args[i].target = target;
        btch_args[i].seed = seed;
        btch_args[i].cancelled = &cancelled;
        btch_args[i].sol_mutex = &sol_mutex;
        btch_args[i].btch_id = i;

        // allocate working space for the thread
        btch_args[i].start = n + i * chunk;
        if(i == thread_count - 1)
            btch_args[i].end = UINT64_MAX;
        else
            btch_args[i].end = n + (i + 1) * chunk;

        if((pthread_create(btches + i, NULL, work_btch, (void*)(btch_args + i))) < 0) {
            perror("ERROR creating thread");
        }
#if DEBUG
        fprintf(stdout, "[ WORKMAN ] Made workbtch at %lu\n", btches[i]);
#endif
    }

    while(solution == 0) {
#if DEBUG
        fprintf(stderr, "[ WORKMAN ] Waiting for solution..\n");
#endif
        sleep(1);
    }

#if DEBUG
    fprintf(stderr, "[ WORKMAN ] Solution found!\n");
#endif

    sem_post(worker_sem);

    // Now wait for its turn to send the message :)
    int curr;
    while((curr = get_tid(tid_queue, queue_mutex)) != thread_id) {
#if DEBUG
        fprintf(stderr, "[ WORKMAN ] Worker %d is waiting for its turn, current is %d\n", thread_id, curr);
#endif
        sleep(1);
    }

    pthread_mutex_lock(arg->worker_mutex);

    char* result = malloc(sizeof(char) * 98);
    char print_seed[65];
    bzero(print_seed, 65);
    memcpy(print_seed, raw_seed, 64);
    difficulty = htonl(difficulty);
    sprintf(result, "SOLN %08x %s %016lx\r\n", difficulty, print_seed, solution);
#if DEBUG
    fprintf(stderr, "[ WORKMAN ] Sending result!\n");
#endif
    send_message(newsockfd, result, 97);

    pthread_mutex_unlock(arg->worker_mutex);

#if DEBUG
    fprintf(stderr, "[ WORKMAN ] Worker %d cleaning up\n", thread_id);
#endif
    pthread_cleanup_pop(1);
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
#if DEBUG
    fprintf(stderr, "[ WORKBTCH ] Btch trying from %lu to %lu (inclusive)\n", start, end);
#endif

    while(1) {
        int res;

        if(*cancelled || trying >= end)
            break;

        if((res = is_valid_soln(target, seed, trying)) == -1) {
#if DEBUG
            fprintf(stderr, "[ WORKBTCH ] Solution found %lu\n", trying);
#endif
            pthread_mutex_lock(mutex);
            *solution = trying;
            // kill siblings.. :(
            *cancelled = 1;
            break;
        }

        trying += 1;
    }

#if DEBUG
    fprintf(stderr, "[ WORKBTCH ] Workbtch %d is now dying\n", arg->btch_id);
#endif
    return 0;
}


