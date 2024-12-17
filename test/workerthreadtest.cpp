#include "infra/workerthread.h"

#include <doctest/doctest.h>
#include <set>
#include <thread>
#include <stdexcept>

namespace inf::test {

using namespace std;
using namespace doctest;

class WorkerThreadTest
{
protected:
    WorkerThreadTest()
    {
        wt.start();
    }

    ~WorkerThreadTest()
    {
        wt.stop();
    }

    WorkerThread wt;
};

TEST_CASE_FIXTURE(WorkerThreadTest, "startTwice")
{
    // first start is in teardown
    CHECK_NOTHROW(wt.start());
}

TEST_CASE_FIXTURE(WorkerThreadTest, "stopTwice")
{
    // second stop is in teardown
    CHECK_NOTHROW(wt.stop());
}

TEST_CASE_FIXTURE(WorkerThreadTest, "addJobWhileStopped")
{
    // second stop is in teardown
    wt.stop();
    CHECK_NOTHROW(wt.add_job([&]() {}));
}

TEST_CASE_FIXTURE(WorkerThreadTest, "RunJobs")
{
    const long jobCount = 10;

    std::mutex mutex;
    std::condition_variable cond;

    uint32_t count = 0;
    std::set<std::thread::id> threadIds;

    for (auto i = 0; i < jobCount; ++i) {
        wt.add_job([&]() {
            std::lock_guard<std::mutex> lock(mutex);
            threadIds.insert(std::this_thread::get_id());
            if (++count == jobCount) {
                cond.notify_one();
            }
        });
    }

    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock, [&]() { return count == jobCount; });

    CHECK(threadIds.size() == 1);
}

TEST_CASE_FIXTURE(WorkerThreadTest, "RunJobThatFails")
{
    std::mutex mutex;
    std::condition_variable cond;
    std::exception_ptr ex;

    wt.ErrorOccurred.connect(this, [&](std::exception_ptr e) {
        ex = e;
        std::lock_guard<std::mutex> lock(mutex);
        cond.notify_one();
    });

    wt.add_job([&]() {
        throw std::runtime_error("Error");
    });

    {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [&]() { return ex != nullptr; });
    }

    wt.stop();

    CHECK_THROWS_AS(std::rethrow_exception(ex), std::runtime_error);
}
}