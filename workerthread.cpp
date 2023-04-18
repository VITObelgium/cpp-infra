#include "infra/workerthread.h"

#include <cassert>
#include <thread>

namespace inf {

class WorkerThread::Task
{
public:
    Task(WorkerThread& worker, std::function<void()> cb)
    : _stop(false)
    , _worker(worker)
    , _startCb(cb)
    , _thread(&Task::run, this)
    {
    }

    ~Task()
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _stop = true;
            _condition.notify_one();
        }

        if (_thread.joinable()) {
            _thread.join();
        }
    }

    void set_stop_cb(std::function<void()> cb)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _stopCb = cb;
    }

    void signal_job_available()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _condition.notify_one();
    }

    void run()
    {
        if (_startCb) {
            _startCb();
            _startCb = nullptr;
        }

        for (;;) {
            std::unique_lock<std::mutex> lock(_mutex);
            if (!_worker.has_jobs() || !_stop) {
                _condition.wait(lock, [this]() { return _worker.has_jobs() || _stop; });
            }

            if (!_worker.has_jobs()) {
                break;
            }

            lock.unlock();

            auto job = _worker.next_job();
            while (job) {
                try {
                    job();
                } catch (...) {
                    _worker.ErrorOccurred(std::current_exception());
                }

                job = _worker.next_job();
            }
        }

        if (_stopCb) {
            _stopCb();
            _stopCb = nullptr;
        }
    }

private:
    bool _stop;
    WorkerThread& _worker;
    std::function<void()> _startCb;
    std::function<void()> _stopCb;

    std::mutex _mutex;
    std::condition_variable _condition;
    std::thread _thread;
};

WorkerThread::WorkerThread()  = default;
WorkerThread::~WorkerThread() = default;

void WorkerThread::start(std::function<void()> cb)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_thread) {
        _thread = std::make_unique<Task>(*this, cb);
    }
}

void WorkerThread::stop(std::function<void()> cb)
{
    clear_jobs();
    stop_finish_jobs(cb);
}

void WorkerThread::stop_finish_jobs(std::function<void()> cb)
{
    if (_thread) {
        _thread->set_stop_cb(cb);
        _thread.reset();
    }

    assert(_jobQueue.empty());
}

void WorkerThread::add_job(const std::function<void()>& job)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _jobQueue.push_back(job);
    }

    if (_thread) {
        _thread->signal_job_available();
    }
}

void WorkerThread::clear_jobs()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _jobQueue.clear();
}

bool WorkerThread::has_jobs()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return !_jobQueue.empty();
}

std::function<void()> WorkerThread::next_job()
{
    std::function<void()> job;

    std::lock_guard<std::mutex> lock(_mutex);
    if (!_jobQueue.empty()) {
        job = _jobQueue.front();
        _jobQueue.pop_front();
    }

    return job;
}

}
