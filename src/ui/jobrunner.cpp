#include "jobrunner.h"

#include "infra/gdal.h"
#include "infra/log.h"

namespace opaq {

using inf::Log;

inf::ThreadPool JobRunner::_pool;

void JobRunner::start(int32_t numThreads)
{
    _pool.set_thread_creation_cb([]() {
        // make sure we can do coordinate transformations in the worker threads
        inf::gdal::registerEmbeddedData();
    });

    _pool.set_thread_destruction_cb([]() {
        inf::gdal::unregisterEmbeddedData();
    });

    _pool.start(numThreads);
    Log::debug("Thread pool started ({} threads)", _pool.thread_count());
}

void JobRunner::stop()
{
    Log::debug("Stopping thread pool");
    _pool.stop_finish_jobs();
    Log::debug("Thread pool stopped");
}

void JobRunner::queue(std::function<void()> func)
{
    _pool.add_job(func);
}

void JobRunner::setUncaughtExceptionCb(void* receiver, std::function<void(std::exception_ptr)> func)
{
    _pool.UncaughtException.connect(receiver, func);
}
}
