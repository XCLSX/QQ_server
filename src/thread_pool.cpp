#include <thread_pool.h>

Mythread_pool::Mythread_pool(int min, int max,int q_max)
{
    thread_shutdown = true;
    thread_min = min;
    thread_max = max;
    thread_busy = 0;
    thread_exitNum = 0;
    thread_alive = 0;
    que_max = q_max;
    tids = new pthread_t[max];
    //初始化锁及条件变量
    if(pthread_mutex_init(&lock,NULL)!=0)
    {
        printf("mutex init failed\n");
    }
    if(pthread_cond_init(&not_full,NULL)!=0||pthread_cond_init(&not_empty,NULL)!=0)
    {
        printf("cond init failed\n");
    }
    for(int i=0;i<min;i++)
    {
        if(pthread_create(&tids[i],NULL,Customer_job,(void *)this) != 0)
        {
            printf("thread_create failed\n");
            return ;
        }

    }
    if(pthread_create(&manger_tid,NULL,Manger_job,(void *)this) != 0)
    {
        printf("create manger thread failed\n");
        return ;
    }

}

void Mythread_pool::AddTask(void *(task)(void *), void *arg)
{
    task_t *tk = new task_t;
    tk->task = task;
    tk->arg = arg;

    pthread_mutex_lock(&lock);
    while(q_task.size() == que_max && thread_shutdown)
    {
        pthread_cond_wait(&not_full,&lock);
    }
    if(!thread_shutdown)
    {
        pthread_mutex_unlock(&lock);
        return ;
    }
    q_task.push(tk);
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&lock);
}

void *Mythread_pool::Customer_job(void *arg)
{
    task_t *task;
    Mythread_pool *pool = (Mythread_pool*)arg;
    while(pool->thread_shutdown)
    {
       pthread_mutex_lock(&pool->lock);
       while(pool->q_task.size() == 0&& pool->thread_shutdown)
       {
           pthread_cond_wait(&pool->not_empty,&pool->lock);
       }
       if(!pool->thread_shutdown)
       {
           pthread_mutex_unlock(&pool->lock);
           pthread_exit(NULL);
       }
       //判断是否需要自杀
       if(pool->thread_exitNum>0&&pool->thread_alive>pool->thread_min)
       {
           --pool->thread_exitNum;
           --pool->thread_alive;
           pthread_mutex_unlock(&pool->lock);
           pthread_exit(NULL);
       }
       task = pool->q_task.front();
       ++pool->thread_busy;
       pool->q_task.pop();
       //唤醒生产者
       pthread_cond_signal(&pool->not_full);
       //解锁
       pthread_mutex_unlock(&pool->lock);
       //运行任务
       (*(task->task))(task->arg);
       pthread_mutex_lock(&pool->lock);
       --pool->thread_busy;
       pthread_mutex_unlock(&pool->lock);
       delete task;
    }
}

void *Mythread_pool::Manger_job(void *arg)
{
    Mythread_pool *pool = (Mythread_pool*)arg;
    int alive;
    int busy;
    int current_task;
    int add;
    while(pool->thread_shutdown)
    {
        add = 0;
        pthread_mutex_lock(&pool->lock);
        alive = pool->thread_alive;
        busy = pool->thread_busy;
        current_task = pool->q_task.size();
        pthread_mutex_unlock(&pool->lock);
        //剩余任务大于闲置线程或者忙线程超过80%进行扩容
        if((current_task > alive - busy || (double)busy/alive>=0.8)&& pool->thread_max > alive)
        {
            for(int i=0;i<pool->thread_max&&add<pool->thread_min;i++)
                if(pool->tids[i] == 0 || if_thread_alive(pool->tids[i]))
                {
                    if(pthread_create(&pool->tids[i],NULL,Customer_job,arg) > 0)
                    {
                        printf("add thread failed\n");
                        continue;
                    }
                    pthread_mutex_lock(&pool->lock);
                    pool->thread_alive++;
                    pthread_mutex_unlock(&pool->lock);
                    add++;
                }
        }
        //如果忙线程少于三分之一缩减
        if(busy*2<alive-busy && alive>pool->thread_min)
        {
            pthread_mutex_lock(&pool->lock);
            pool->thread_exitNum = 10;
            pthread_mutex_unlock(&pool->lock);
            for(int i=0;i<10;i++)
            {
                pthread_cond_signal(&pool->not_empty);
            }
        }
        sleep(10);
    }
}

int Mythread_pool::if_thread_alive(pthread_t tid)
{
    if(kill(tid,0) == -1)
    {
        if(errno == ESRCH)
            return false;
    }
    return true;
}

