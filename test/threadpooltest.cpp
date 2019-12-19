#include "infra/threadpool.h"

#include <chrono>
#include <doctest/doctest.h>
#include <future>
#include <thread>
#include <unordered_set>

namespace inf::test {

using namespace std;

constexpr int g_poolSize = 4;

class ThreadPoolTest
{
protected:
    ThreadPoolTest()
    {
        tp.start(g_poolSize);
    }

    ~ThreadPoolTest()
    {
        tp.stop();
    }

    ThreadPool tp;
};

TEST_CASE_FIXTURE(ThreadPoolTest, "startTwice")
{
    // first start is in teardown
    CHECK_NOTHROW(tp.start(g_poolSize));
}

TEST_CASE_FIXTURE(ThreadPoolTest, "stopTwice")
{
    // second stop is in teardown
    CHECK_NOTHROW(tp.stop());
}

TEST_CASE_FIXTURE(ThreadPoolTest, "RunJobs")
{
    const uint32_t jobCount = g_poolSize * 100;

    std::mutex mutex;
    std::condition_variable cond;

    uint32_t count = 0;
    std::unordered_set<std::thread::id> threadIds;

    for (auto i = 0u; i < jobCount; ++i) {
        tp.add_job([&]() {
            std::unique_lock<std::mutex> lock(mutex);
            threadIds.insert(std::this_thread::get_id());

            if (count < g_poolSize) {
                // make sure all the jobs in the pool are used before finishing this task
                cond.wait_for(lock, std::chrono::milliseconds(10));
            }

            if (++count == jobCount) {
                cond.notify_one();
            }
        });
    }

    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock, [&]() { return count == jobCount; });

    CHECK(g_poolSize == static_cast<uint32_t>(threadIds.size()));
}

TEST_CASE_FIXTURE(ThreadPoolTest, "StopFinishJobs")
{
    const long jobCount = g_poolSize * 100;

    std::mutex mutex;

    long count = 0;
    for (auto i = 0; i < jobCount; ++i) {
        tp.add_job([&]() {
            std::unique_lock<std::mutex> lock(mutex);
            ++count;
        });
    }

    tp.stop_finish_jobs();
    CHECK(jobCount == count);
}

TEST_CASE_FIXTURE(ThreadPoolTest, "ErrorInjob")
{
    std::promise<void> prom;

    tp.UncaughtException.connect(this, [&](std::exception_ptr ex) {
        prom.set_exception(ex);
    });

    tp.add_job([]() {
        throw std::runtime_error("Oops");
    });

    auto fut = prom.get_future();
    CHECK(std::future_status::ready == fut.wait_for(std::chrono::seconds(3)));
    CHECK_THROWS_AS(fut.get(), std::runtime_error);

    tp.stop_finish_jobs();
}

TEST_CASE_FIXTURE(ThreadPoolTest, "StartStopFinishJobs")
{
    CHECK_NOTHROW(tp.stop_finish_jobs());
}

TEST_CASE_FIXTURE(ThreadPoolTest, "CallInitDestroyFunctions")
{
    tp.stop_finish_jobs();

    std::atomic<int> startThreadCounter = 0;
    std::atomic<int> stopThreadCounter  = 0;

    tp.set_thread_creation_cb([&]() {
        ++startThreadCounter;
    });

    tp.set_thread_destruction_cb([&]() {
        ++stopThreadCounter;
    });

    tp.start(g_poolSize);
    tp.stop_finish_jobs();
    CHECK(g_poolSize == startThreadCounter);
    CHECK(g_poolSize == stopThreadCounter);
}
}
