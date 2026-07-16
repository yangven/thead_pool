#include "TaskQueue.h"

class ThreadPool{
public:
    ThreadPool(int main, int max);
    ~ThreadPool();    
    
    //添加任务
    void addTask(Task task);
    //添加忙线程的个数
    int getBusyNumber();
    //获取活着线程个数
    int getAliveNumber();

private:
    //工作线程的任务函数
    static void* worker(void* arg);
    //管理者线程的任务函数
    static void* manager(void* arg);
    void threadExit();

private:
    pthread_mutex_t m_lock;
    pthread_cond_t m_notEmpty;
    pthread_t* m_threadIDs;
    pthread_t m_managerID;
    TaskQueue* m_taskQ;
    int m_minNum;
    int m_maxNum;
    int m_busyNum;
    int m_aliveNum;
    int m_exitNum;
    bool m_shutdown =false;
};