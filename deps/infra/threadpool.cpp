#include "infra/threadpool.h"

#include <cassert>
#include <thread>

namespace inf {

class ThreadPool::Task
{
public:
    Task(ThreadPool& pool)
    : _stop(false)
    , _stopFinish(false)
    , _pool(pool)
    , _thread(&Task::run, this)
    {
    }

    ~Task()
    {
        assert(_thread.joinable());
        _thread.join();
    }

    void stop()
    {
        _stop = true;
    }

    void stop_finish_jobs()
    {
        _stopFinish = true;
    }

    void run()
    {
        if (_pool._threadCreateCb) {
            _pool._threadCreateCb();
        }

        for (;;) {
            std::unique_lock<std::mutex> lock(_pool._poolMutex);
            if (!_pool.has_jobs() || !_stop) {
                _pool._condition.wait(lock, [this]() { return _pool.has_jobs() || _stop || _stopFinish; });
            }

            if (_stop || (_stopFinish && !_pool.has_jobs())) {
                break;
            }

            lock.unlock();

            auto job = _pool.get_job();
            while (job && !_stop) {
                try {
                    job();
                } catch (...) {
                    _pool.UncaughtException(std::current_exception());
                }

                job = _pool.get_job();
            }
        }

        if (_pool._threadDestroyCb) {
            _pool._threadDestroyCb();
        }
    }

private:
    bool _stop;
    bool _stopFinish;
    ThreadPool& _pool;

    std::thread _thread;
};

ThreadPool::ThreadPool()  = default;
ThreadPool::~ThreadPool() = default;

size_t ThreadPool::thread_count() const
{
    return _threads.size();
}

void ThreadPool::set_thread_creation_cb(std::function<void()> cb)
{
    _threadCreateCb = cb;
}

void ThreadPool::set_thread_destruction_cb(std::function<void()> cb)
{
    _threadDestroyCb = cb;
}

void ThreadPool::start(int numThreads)
{
    std::lock_guard<std::mutex> lock(_poolMutex);

    if (!_threads.empty()) {
        return;
    }

    for (auto i = 0; i < numThreads; ++i) {
        _threads.push_back(std::make_unique<Task>(*this));
    }
}

void ThreadPool::stop()
{
    {
        std::lock_guard<std::mutex> lock(_jobsMutex);
        _queuedJobs.clear();
    }

    {
        std::lock_guard<std::mutex> lock(_poolMutex);
        for (auto& t : _threads) {
            t->stop();
        }
        _condition.notify_all();
    }

    // Will cause joining of the threads
    _threads.clear();
}

void ThreadPool::stop_finish_jobs()
{
    {
        std::lock_guard<std::mutex> lock(_poolMutex);
        for (auto& t : _threads) {
            t->stop_finish_jobs();
        }
        _condition.notify_all();
    }

    // Will cause joining of the threads
    _threads.clear();
}

bool ThreadPool::has_jobs()
{
    std::lock_guard<std::mutex> lock(_jobsMutex);
    return !_queuedJobs.empty();
}

void ThreadPool::add_job(std::function<void()> job)
{
    {
        std::lock_guard<std::mutex> lock(_jobsMutex);
        _queuedJobs.push_back(job);
    }

    {
        std::lock_guard<std::mutex> lock(_poolMutex);
        _condition.notify_one();
    }
}

std::function<void()> ThreadPool::get_job()
{
    std::function<void()> job;

    {
        std::lock_guard<std::mutex> lock(_jobsMutex);
        if (!_queuedJobs.empty()) {
            job = _queuedJobs.front();
            _queuedJobs.pop_front();
        }
    }

    return job;
}
}
