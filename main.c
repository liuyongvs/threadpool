/********************************************
 *@author    JackyLau
 *@date      10/26/2013
 *
 *
 * ******************************************/

#include <stdio.h>
#include <unistd.h>
#include"threadpool.h"

void task1()
{
    printf("thread %d running Task 1\n", (int)pthread_self());
    int i;
    /*for delay*/
    int sum = 0;
    for(i = 0; i < 100000; i++)
    {
        sum += i;
    }
}

void task2(int id)
{
    printf("%d thread %d running Task 2\n", id, (int)pthread_self());
    /*for delay*/
    int i;
    int sum = 0;
    for(i = 0; i < 100000; i++)
    {
        sum += i;
    }
}

int main(void)
{
    threadpool_t *threadpool = threadpool_init(10);
    int i;
    for(i = 0; i < 1000; i++)
    {
        threadpool_add_work(threadpool, (void *)task1, NULL);
        threadpool_add_work(threadpool, (void *)task2, (void *)i);
    }

    sleep(5);/*so they can handle all of the jobs, or just a bit of them*/
    threadpool_destroy(threadpool);

    //printf("Hello World!\n");
    return 0;
}

