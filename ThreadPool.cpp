//线程池函数的类定义
#include "ThreadPoll.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
using namespace std;

ThreadPool::ThreadPool (int min, int max){
    //实例化任务队列
    m_taskQ = new TaskQueue;
    do {
        //初始化线程池
        m_minNum = min;
        m_maxNum = max;
        m_busyNum = 0;
        m_aliveNum = min;
        m_exitNum = 0;

        //根据线程最大上限给线程组分配内存
        m_threadIDs = new pthread_t[max];
        if (m_threadIDs == nullptr) {
            cout << "new thread_t[] 失败..." << endl;
            break;
        }
        //初始化
        memset(m_threadIDs, 0, sizeof(pthread_t)*max);
        //初始化互斥锁，条件变量
        if(pthread_mutex_init(&m_lock, NULL) !=0 || pthread_cond_init(&m_notEmpty, NULL) != 0){
            cout << "init mutex or condition fail..." << endl;
            break;
            }
        
        // 创建线程
        //根据最小线程个数，创建线程
        for (int i = 0; i < min; ++i){
            pthread_create(&m_threadIDs[i], NULL, worker, this);
            cout << "创建子线程， ID:" << to_string(m_threadIDs[i]) << endl;
        }
        //创建管理者线程
        pthread_create(&m_managerID, NULL, manager, this);
    } while(0);
}

//析构函数
ThreadPool::~ThreadPool(){
    m_shutdown = true;
    //销毁管理者线程
    pthread_join(m_managerID, NULL);
    //唤醒所有消费者线程
    for(int i = 0; i < m_aliveNum; ++i){
        pthread_cond_signal(&m_notEmpty);
    }
    if (m_taskQ) delete m_taskQ;
    if (m_threadIDs) delete[]m_threadIDs;
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_notEmpty);
}

//工作线程任务函数
void* ThreadPool::worker(void* arg){
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    //持续工作
    while(true){
        //访问任务队列
        pthread_mutex_lock(&pool->m_lock);
        //判断任务队列是否为空，如果为空工作线程阻塞
        while(pool->m_taskQ->taskNumber() == 0 && !pool->m_shutdown){
            cout << "thead" << to_string(pthread_self()) << "wait..." << endl;
            //阻塞线程
            pthread_cond_wait(&pool->m_notEmpty, &pool->m_lock);
            //接触阻塞线程后，判断是否需要销毁线程
            if(pool->m_exitNum > 0){
                pool->m_exitNum--;
                if(pool->m_aliveNum > pool->m_minNum){
                    pool->m_aliveNum--;
                    pthread_mutex_unlock(&pool->m_lock);
                    pool->threadExit();
                }
            }
        }
        //判断线程是否关闭
        if (pool->m_shutdown){
            pthread_mutex_unlock(&pool->m_lock);
            pool->threadExit();
        }

        //从任务队列中取出一个任务
        Task task = pool->m_taskQ->takeTask();
        //工作忙线程+1
        pool->m_busyNum++;
        //线程池解锁
        pthread_mutex_unlock(&pool->m_lock);
        //执行任务
        cout << "thread" << to_string(pthread_self()) << "start working..." << endl;
        task.function(task.arg);
        task.arg = nullptr;

        //任务处理结束
        cout << "thread" << to_string(pthread_self()) << "end working..." << endl;
        pthread_mutex_lock(&pool->m_lock);
        pool->m_busyNum--;
        pthread_mutex_unlock(&pool->m_lock);
    }
}

//管理者线程任务函数
void* ThreadPool::manager(void* arg){
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    //线程池未关闭就一直检测
    while (!pool->m_shutdown){
        //5秒检测一次
        sleep(5);
        //取出线程池中的任务数和线程数
        //取出工作的线程数量
        pthread_mutex_lock(&pool->m_lock);
        int queueSize = pool->m_taskQ->taskNumber();
        int liveNum = pool->m_aliveNum;
        int busyNum = pool->m_busyNum;
        pthread_mutex_unlock(&pool->m_lock);

        //创建线程
        const int NUMBER = 2;
        //当前任务个数大于线程个数，存活线程小于最大线程
        if (queueSize > liveNum && liveNum <pool->m_maxNum){
            //线程池枷锁
            pthread_mutex_lock(&pool->m_lock);
            int num = 0;
            for(int i = 0; i<pool->m_maxNum && num<NUMBER && pool->m_aliveNum<pool->m_maxNum; ++i){
                if(pool->m_threadIDs[i] == 0){
                    pthread_create(&pool->m_threadIDs[i],NULL, worker, pool);
                    num++;
                    pool->m_aliveNum++;
                }
            }
            pthread_mutex_unlock(&pool->m_lock);
        }

        //摧毁多余线程
        if (busyNum*2 < liveNum && liveNum > pool->m_minNum){
            pthread_mutex_lock(&pool->m_lock);
            pool->m_exitNum = NUMBER;
            pthread_mutex_unlock(&pool->m_lock);
            for (int i = 0; i<NUMBER; ++i){
                pthread_cond_signal(&pool->m_notEmpty);
            }
        }

    }
    return nullptr;
}

//添加任务
void ThreadPool::addTask(Task task){
    if (m_shutdown){
        return;
    }
    //添加任务，不需要加锁，任务队列中提前已经加锁
    m_taskQ->addTask(task);
    //唤醒工作线程
    pthread_cond_signal(&m_notEmpty);
}


int ThreadPool::getAliveNumber(){
    pthread_mutex_lock(&m_lock);
    int threadNum = m_aliveNum;
    pthread_mutex_unlock(&m_lock);
    return threadNum;
}


int ThreadPool::getBusyNumber(){
    pthread_mutex_lock(&m_lock);
    int busyNum = m_busyNum;
    pthread_mutex_unlock(&m_lock);
    return busyNum;
}


//线程退出
void ThreadPool::threadExit(){
    pthread_t tid = pthread_self();
    for(int i = 0; i<m_maxNum; ++i){
        if (m_threadIDs[i] == tid){
            cout << "threadExit() function: thrad" << to_string(pthread_self()) << "exiting..." << endl;
            m_threadIDs[i] = 0;            
        }
    }
    pthread_exit(NULL);
}

