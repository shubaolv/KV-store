#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <pthread.h>

typedef void *(*callback_func)(void*);

typedef struct job
{
    callback_func p_callback_func;
    void *arg;
    struct job *next;
} job_t;

typedef struct threadpool
{
    int thread_num;
    int queue_max_num;
    job_t *head;
    job_t *tail;
    pthread_t *pthreads;
    pthread_mutex_t mutex;
    pthread_cond_t queue_empty;
    pthread_cond_t queue_not_empty;
    pthread_cond_t queue_not_full;
    int queue_cur_num;
    int queue_close;
    int pool_close;
} threadpool_t;

threadpool_t *threadpool_init(int thread_num, int queue_max_num);

int threadpool_add_job(threadpool_t *pool, callback_func p_callback_fun, void *arg);

int threadpool_destory(threadpool_t *pool);

void *threadpool_function(void *arg);

#endif /* _THREAD_POOL_H_ */
