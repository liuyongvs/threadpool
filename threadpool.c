#include"threadpool.h"
#include<stdio.h>
#include<malloc.h>
#include<stdlib.h>
#include<assert.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
typedef int bool;
#define true  1
#define false 0
bool thread_islive = true;

#define CHECK_ERROR(a)                            \
if((a))                                           \
{                                                 \
    perror("Error at line\n\t" #a "\nSystem Msg");\
    exit(EXIT_FAILURE);                           \
}

/*==================FUNCTIONS========================*/

threadpool_t *threadpool_init(int num_threads)
{
    if(num_threads < 1)
        num_threads =1;
    /* malloc the struct threadpool*/
    threadpool_t *threadpool = (threadpool_t *) malloc(sizeof(threadpool_t));

    /*malloc the threads' ID */
    threadpool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    threadpool->num_threads = num_threads;

    /*initialize the workqueue*/
    threadpool->head = NULL;
    threadpool->tail = NULL;
    threadpool->num_jobs = 0;

    /*initialize the semaphore*/
    threadpool->queue_sem = (sem_t *)malloc(sizeof(sem_t));
    sem_init(threadpool->queue_sem, 0, 0);

    /*make the threads in pool*/
    int i;
    for(i = 0; i < num_threads; i++)
    {
        pthread_create(&threadpool->threads[i], NULL, (void *)threadpool_thread_do, (void *)threadpool);
    }
    return threadpool;
}

/*销毁线程池，等待队列中的任务不会再被执行，但是正在运行的线程会一直
把任务运行完后再退出*/

void threadpool_destroy(threadpool_t *threadpool)
{
    thread_islive = false;
    int i;
    /*because create threads more than jobs, the threads will be zoombie*, so wake all of them*/
    for(i = 0; i < threadpool->num_threads; i++)
    {
        CHECK_ERROR(sem_post(threadpool->queue_sem));
    }
    CHECK_ERROR(sem_destroy(threadpool->queue_sem));

    /*barrier for synchronization*/
    for(i = 0; i < threadpool->num_threads; i++)
        pthread_join(threadpool->threads[i], NULL);

    /*dealloc job queue*/
    /*if the jobs is so many and can't handle them, it will cause mem leak, so empty the queue*/
    threadpool_job_queue_empty(threadpool);
    /*dealloc threads*/
    free(threadpool->threads);
    /*dealloc semphore*/
    free(threadpool->queue_sem);
    /*dealloc threadpool*/
    free(threadpool);

}

int threadpool_job_queue_empty(threadpool_t *threadpool)
{
    thread_job_t *prev = threadpool->head;
    thread_job_t *p = prev;
    while(threadpool->num_jobs)
    {
        p = p->next;
        free(prev);
        prev = p;
        threadpool->num_jobs--;
    }
    threadpool->head = NULL;
    threadpool->tail = NULL;
    return 0;
}

void threadpool_thread_do(threadpool_t *threadpool_in)
{
    assert(threadpool_in);
    threadpool_t *threadpool = threadpool_in;

    if(sem_wait(threadpool->queue_sem))
    {
        perror("thread waiting for semaphore");
        exit(EXIT_FAILURE);
    }

    while(thread_islive)
    {
        pthread_mutex_lock(&mutex);     /* LOCK */

        thread_job_t *job= threadpool_job_dequeue(threadpool);

        pthread_mutex_unlock(&mutex);    /* UNLOCK */

        if(job)
        {
            function_t pf = job->pf;
            void *arg = job->arg;
            pf(arg);
            free(job);
        }
    }
}

thread_job_t *threadpool_job_dequeue(threadpool_t *threadpool)
{
    assert(threadpool);

    thread_job_t *job = threadpool->head;
    switch (threadpool->num_jobs) {
    case 0:
        job = NULL;
        break;
    case 1:
        threadpool->head = NULL;
        threadpool->tail = NULL;
        threadpool->num_jobs--;
        break;
    default:
        threadpool->head = job->next;
        job->next = NULL;
        threadpool->head->prev = NULL;
        threadpool->num_jobs--;
        break;
    }
    return job;
}

int threadpool_add_work(threadpool_t *threadpool, function_t pf, void *arg)
{
    assert(threadpool);
    thread_job_t *job = (thread_job_t *)malloc(sizeof(thread_job_t));
    job->pf = pf;
    job->arg = arg;
    job->next = NULL;
    job->prev = NULL;

    pthread_mutex_lock(&mutex);          /* LOCK */

    CHECK_ERROR( threadpool_job_enqueue(threadpool, job) );

    pthread_mutex_unlock(&mutex);         /* UNLOCK */
    return 0;
}

int threadpool_job_enqueue(threadpool_t *threadpool, thread_job_t *job)
{
    assert(threadpool);
    assert(job);
    switch(threadpool->num_jobs)
    {
    case 0:
        threadpool->head = job;
        threadpool->tail = job;
        threadpool->num_jobs++;
        break;
    default:
        threadpool->tail->next = job;
        job->prev = threadpool->tail;
        threadpool->tail = job;
        threadpool->num_jobs++;
    }

    sem_post(threadpool->queue_sem);
    return 0;
}
