//线程池函数的类定义
#include "ThreadPoll.h"
#include <iostream>
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

        //根据线程最大上限给线程组分配内存
        m_threadIDs = new pthread_t[max];
        if (m_threadIDs = nullptr) {
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
    
}

//工作线程任务函数
void* ThreadPool::worker(void* arg){

}

//管理者线程任务函数
void* ThreadPool::manager(void* arg){
    
}

//添加任务
void ThreadPool::addTask(Task task){

}

