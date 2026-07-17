#include "TaskQueue.h"

// 线程池类：负责管理工作线程、管理者线程以及待执行的任务队列。
class ThreadPool{
public:
    // 创建线程池。
    // min：线程池中最少保留的工作线程数
    // max：线程池允许创建的最大工作线程数。
    ThreadPool(int min, int max);

    ~ThreadPool();
    
    // 向任务队列中添加一个待执行任务，并唤醒等待任务的工作线程。
    void addTask(Task task);

    // 获取当前正在执行任务的工作线程数量。
    int getBusyNumber();

    // 获取当前仍然存活的工作线程数量，包括忙线程和空闲线程。
    int getAliveNumber();

private:
    // 工作线程入口函数。
    // 工作线程循环等待任务，从任务队列中取出任务并执行。
    // 声明为 static 是为了符合 pthread_create 要求的函数签名。
    // arg 通常传入当前 ThreadPool 对象的 this 指针。
    static void* worker(void* arg);

    // 管理者线程入口函数。
    // 根据任务数量和忙线程数量，动态创建或销毁部分工作线程。
    static void* manager(void* arg);

    // 结束当前工作线程，并同步更新线程 ID 和存活线程数量。
    void threadExit();

private:
    pthread_mutex_t m_lock;       // 保护线程池共享状态，如忙线程数、存活线程数等。
    pthread_cond_t m_notEmpty;    // “任务队列非空”条件变量，用于唤醒等待任务的工作线程。
    pthread_t* m_threadIDs;       // 工作线程 ID 数组，数组容量通常为 m_maxNum。
    pthread_t m_managerID;        // 管理者线程的 ID。
    TaskQueue* m_taskQ;           // 任务队列，保存尚未被工作线程执行的任务。
    int m_minNum;                 // 线程池需要维持的最少工作线程数量。
    int m_maxNum;                 // 线程池允许存在的最大工作线程数量。
    int m_busyNum;                // 当前正在执行任务的工作线程数量。
    int m_aliveNum;               // 当前存活的工作线程总数。
    int m_exitNum;                // 管理者线程要求退出的工作线程数量。
    bool m_shutdown = false;      // 线程池关闭标志：true 表示正在或已经关闭。
};
