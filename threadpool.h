/**********************************************
  * @author Jacky Lau
  * @date   10/26/2013
  *
  *
  * **********************************************/

#include<pthread.h>
#include<semaphore.h>


/*========================STRUCTS==========================*/

typedef void *(*function_t)(void *arg);

/* jobs */
typedef struct thread_job_t{
    function_t pf;      /**< function pointer        */
    void *arg;          /**< function's pointer      */
    struct thread_job_t *next; /**< pointer to next job     */
    struct thread_job_t*prev; /**< pointer to previous job */
}thread_job_t;


/* threadpool */
typedef struct threadpool_t{
    pthread_t *threads; /**< pointer to thread's ID */
    int num_threads;    /**< number of threads      */

    thread_job_t *head; /**< pointer to head of workqueue */
    thread_job_t *tail; /**< pointer to tail of workqueue */
    int num_jobs;       /**< number of jobs in the workqueue*/
    sem_t *queue_sem;

}threadpool_t;

/*==========================FUNCTION=================================*/

/*-------------------------threadpool specific------------------------------- */

/**
 * @brief initialize threadpool
 * Allocates memory for the threadpool, workqueue,semphore and fixes pointer
 * in job queue
 * @param num_threads to be used
 * @return pointer to threadpool_t struct on success,
 *         NULL on error
 */
threadpool_t *threadpool_init(int num_threads);


/**
 * @brief destroy threadpool
 * if there are so many jobs and can't handle all of them, free the job_queue
 * if there are so many threads, wake all of them in case of zoombie
 * @param threadpool
 */
void threadpool_destroy(threadpool_t *threadpool);

/*-------------------------work specific--------------------------------- */

/**
 * @brief do the jobs, all of the threads are blocked when they first created
 * @param threadpool_in
 */
void threadpool_thread_do(threadpool_t *threadpool_in);

/**
 * @brief add the job into the job_queue, it will wake the blocked threads by sem_post
 * @param threadpool
 * @param pointer to the job function
 * @param argument's of function
 * @return 0 on success,other on failure
 */
int threadpool_add_work(threadpool_t *threadpool, function_t pf, void *arg);


/*-------------------------queue specific--------------------------------- */

/**
 * @brief add job into the queue
 * @param threadpool
 * @param pointer to the struct of thread_job_t
 * @return 0 on success,
 *         other on failure
 */
int threadpool_job_enqueue(threadpool_t *threadpool, thread_job_t *job);

/**
 * @brief remove the first data unit of queue
 * @param threadpool
 * @return 0 on success
 *         other on failure
 */
thread_job_t *threadpool_job_dequeue(threadpool_t *threadpool);

/**
 * @brief make the job_queue empty
 * @param threadpool
 * @return 0 on success
 *         other on failure
 */
int threadpool_job_queue_empty(threadpool_t *threadpool);
