#pragma once

#include <condition_variable>
#include <deque>
#include <memory>

#include "infra/signal.h"

namespace inf {

class WorkerThread
{
public:
    WorkerThread();
    ~WorkerThread();
    WorkerThread(const WorkerThread&)            = delete;
    WorkerThread& operator=(const WorkerThread&) = delete;

    /* Pass a callback that will get executed at the start of the worker thread */
    void start(std::function<void()> cb = nullptr);
    void stop(std::function<void()> cb = nullptr);
    void stop_finish_jobs(std::function<void()> cb = nullptr);

    void add_job(const std::function<void()>& job);

    Signal<std::exception_ptr> ErrorOccurred;

private:
    class Task;

    bool has_jobs();
    std::function<void()> next_job();
    void clear_jobs();

    std::deque<std::function<void()>> _jobQueue;
    std::mutex _mutex;
    std::unique_ptr<Task> _thread;

    friend class Task;
};

}
