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

    void stopFinishJobs()
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
            if (!_pool.hasJobs() || !_stop) {
                _pool._condition.wait(lock, [this]() { return _pool.hasJobs() || _stop || _stopFinish; });
            }

            if (_stop || (_stopFinish && !_pool.hasJobs())) {
                break;
            }

            lock.unlock();

            auto job = _pool.getJob();
            while (job && !_stop) {
                try {
                    job();
                } catch (...) {
                    _pool.UncaughtException(std::current_exception());
                }

                job = _pool.getJob();
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

size_t ThreadPool::threadCount() const
{
    return _threads.size();
}

void ThreadPool::setThreadCreationCb(std::function<void()> cb)
{
    _threadCreateCb = cb;
}

void ThreadPool::setThreadDestructionCb(std::function<void()> cb)
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

void ThreadPool::stopFinishJobs()
{
    {
        std::lock_guard<std::mutex> lock(_poolMutex);
        for (auto& t : _threads) {
            t->stopFinishJobs();
        }
        _condition.notify_all();
    }

    // Will cause joining of the threads
    _threads.clear();
}

bool ThreadPool::hasJobs()
{
    std::lock_guard<std::mutex> lock(_jobsMutex);
    return !_queuedJobs.empty();
}

void ThreadPool::addJob(std::function<void()> job)
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

std::function<void()> ThreadPool::getJob()
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
