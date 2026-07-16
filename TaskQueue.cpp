#include "TaskQueue.h"

//类声明
//构造函数
TaskQueue::TaskQueue(){
    pthread_mutex_init(&m_mutex, NULL);
}

//析构函数
TaskQueue::~TaskQueue(){
    pthread_mutex_destroy(&m_mutex);
}

void TaskQueue::addTask(Task task){
     pthread_mutex_lock(&m_mutex);
     m_taskQ.push(task);
     pthread_mutex_unlock(&m_mutex);
}

//addtask 重载
void TaskQueue::addTask(callback f, void* arg){
    pthread_mutex_lock(&m_mutex);
    m_taskQ.push(Task(f, arg));
    pthread_mutex_unlock(&m_mutex);
}

Task TaskQueue::takeTask(){
    Task t;
    if (!m_taskQ.empty()){
        t = m_taskQ.front();
        m_taskQ.pop();
    }
    return t;
}