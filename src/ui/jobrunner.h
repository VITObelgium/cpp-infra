#pragma once

#include "infra/threadpool.h"

namespace opaq {

class JobRunner
{
public:
    static void start(int32_t numThreads);
    static void stop();

    static void queue(std::function<void()> func);
    static void setUncaughtExceptionCb(void* receiver, std::function<void(std::exception_ptr)> func);

private:
    static inf::ThreadPool _pool;
};
}
