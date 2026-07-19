#include "ThreadPoll.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

namespace {

std::atomic<int> completedTasks{0};

void runTask(void* arg)
{
    const int taskId = *static_cast<int*>(arg);
    delete static_cast<int*>(arg);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "task " << taskId << " finished" << std::endl;
    completedTasks.fetch_add(1, std::memory_order_relaxed);
}

} // namespace

int main()
{
    constexpr int taskCount = 8;
    ThreadPool pool(2, 4);

    for (int i = 0; i < taskCount; ++i) {
        pool.addTask(Task(runTask, new int(i)));
    }

    const auto deadline = std::chrono::steady_clock::now()
                        + std::chrono::seconds(5);
    while (completedTasks.load(std::memory_order_relaxed) < taskCount
           && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    if (completedTasks.load(std::memory_order_relaxed) != taskCount) {
        std::cerr << "test failed: not all tasks completed in time" << std::endl;
        return 1;
    }

    // Give the workers time to finish their bookkeeping before pool destruction.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "test passed: all " << taskCount << " tasks completed"
              << std::endl;
    return 0;
}
