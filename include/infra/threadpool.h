#pragma once

#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <vector>

#include "infra/signal.h"

namespace inf {

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    size_t threadCount() const;

    /* Set a callback that will get exectuded at the start of each thread in the pool */
    void setThreadCreationCb(std::function<void()> cb);
    /* Set a callback that will get exectuded at the end of each thread in the pool */
    void setThreadDestructionCb(std::function<void()> cb);

    void start(int numThreads);
    void stop();
    void stopFinishJobs();
    void addJob(std::function<void()> job);

    Signal<std::exception_ptr> UncaughtException;

private:
    bool hasJobs();
    std::function<void()> getJob();

    class Task;
    friend class Task;

    std::mutex _jobsMutex;
    std::mutex _poolMutex;
    std::condition_variable _condition;
    std::deque<std::function<void()>> _queuedJobs;
    std::vector<std::unique_ptr<Task>> _threads;

    std::function<void()> _threadCreateCb;
    std::function<void()> _threadDestroyCb;
};
}
