#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <queue>
using namespace  std;
struct task_t
{
    void *(*task)(void *);
    void* arg;
};
class Mythread_pool
{
public:
    Mythread_pool(int min,int max,int que_max);
    void AddTask(void*(*task)(void *),void*);
    static void *Customer_job(void *arg);
    static void *Manger_job(void *arg);
    static int if_thread_alive(pthread_t);

private:
    //开关
    bool thread_shutdown;
    int thread_min;
    int thread_max;
    int thread_alive;
    int thread_busy;
    int thread_exitNum;//自杀线程数
    //任务队列最大值
    int que_max;
    queue<task_t*> q_task;
    //锁以及条件变量
    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
    //管理者线程和线程数组
    pthread_t manger_tid;
    pthread_t *tids;

};

#endif // THREAD_POOL_H
