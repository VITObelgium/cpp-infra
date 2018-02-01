#include "infra/threadpool.h"

#include <chrono>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>

namespace infra::test {

using namespace std;
using namespace testing;

constexpr int g_poolSize = 4;

class ThreadPoolTest : public Test
{
protected:
    ThreadPoolTest() = default;

    void SetUp()
    {
        tp.start(g_poolSize);
    }

    void TearDown()
    {
        tp.stop();
    }

    ThreadPool tp;
};

TEST_F(ThreadPoolTest, startTwice)
{
    // first start is in teardown
    EXPECT_NO_THROW(tp.start(g_poolSize));
}

TEST_F(ThreadPoolTest, stopTwice)
{
    // second stop is in teardown
    EXPECT_NO_THROW(tp.stop());
}

TEST_F(ThreadPoolTest, RunJobs)
{
    const long jobCount = g_poolSize * 100;

    std::mutex mutex;
    std::condition_variable cond;

    uint32_t count = 0;
    std::set<std::thread::id> threadIds;

    for (auto i = 0; i < jobCount; ++i) {
        tp.addJob([&]() {
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

    EXPECT_EQ(g_poolSize, static_cast<uint32_t>(threadIds.size()));
}

TEST_F(ThreadPoolTest, StopFinishJobs)
{
    const long jobCount = g_poolSize * 100;

    std::mutex mutex;

    long count = 0;
    for (auto i = 0; i < jobCount; ++i) {
        tp.addJob([&]() {
            std::unique_lock<std::mutex> lock(mutex);
            ++count;
        });
    }

    tp.stopFinishJobs();
    EXPECT_EQ(jobCount, count);
}

TEST_F(ThreadPoolTest, ErrorInjob)
{
    std::promise<void> prom;

    tp.UncaughtException.connect(this, [&](std::exception_ptr ex) {
        prom.set_exception(ex);
    });

    tp.addJob([]() {
        throw std::runtime_error("Oops");
    });

    auto fut = prom.get_future();
    EXPECT_EQ(std::future_status::ready, fut.wait_for(std::chrono::seconds(3)));
    EXPECT_THROW(fut.get(), std::runtime_error);

    tp.stopFinishJobs();
}

TEST_F(ThreadPoolTest, StartStopFinishJobs)
{
    EXPECT_NO_THROW(tp.stopFinishJobs());
}

TEST_F(ThreadPoolTest, CallInitDestroyFunctions)
{
    tp.stopFinishJobs();

    std::atomic<int> startThreadCounter = 0;
    std::atomic<int> stopThreadCounter  = 0;

    tp.setThreadCreationCb([&]() {
        ++startThreadCounter;
    });

    tp.setThreadDestructionCb([&]() {
        ++stopThreadCounter;
    });

    tp.start(g_poolSize);
    tp.stopFinishJobs();
    EXPECT_EQ(g_poolSize, startThreadCounter);
    EXPECT_EQ(g_poolSize, stopThreadCounter);
}
}
