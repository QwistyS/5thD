// thread_pool.h

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <functional>
#include <queue>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    template<class F, class... Args>
    void enqueue(F&& f, Args&&... args);

private:
    // Need to keep track of threads so we can join them
    std::vector<std::thread> workers;

    // The task queue
    std::queue<std::function<void()>> tasks;

    // Synchronization
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

#endif /* THREAD_POOL_H */
