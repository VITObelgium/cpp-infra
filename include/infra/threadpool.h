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
    ThreadPool(const ThreadPool&)            = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    size_t thread_count() const;

    /* Set a callback that will get exectuded at the start of each thread in the pool */
    void set_thread_creation_cb(std::function<void()> cb);
    /* Set a callback that will get exectuded at the end of each thread in the pool */
    void set_thread_destruction_cb(std::function<void()> cb);

    void start(uint32_t numThreads);
    void stop();
    void stop_finish_jobs();
    void add_job(std::function<void()> job);

    Signal<std::exception_ptr> UncaughtException;

private:
    bool has_jobs();
    std::function<void()> get_job();

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
