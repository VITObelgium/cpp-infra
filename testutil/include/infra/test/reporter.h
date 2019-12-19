#pragma once

#include "infra/log.h"

#include <doctest/doctest.h>

namespace inf::test {

using namespace doctest;

struct TestReporter : public IReporter
{
    // caching pointers/references to objects of these types - safe to do
    std::ostream& stdout_stream;
    const ContextOptions& opt;
    const TestCaseData* tc;
    std::mutex mutex;

    // constructor has to accept the ContextOptions by ref as a single argument
    TestReporter(const ContextOptions& in)
    : stdout_stream(*in.cout)
    , opt(in)
    {
    }

    void report_query(const QueryData& /*in*/) override
    {
    }

    void test_run_start() override
    {
    }

    void test_run_end(const TestRunStats& /*in*/) override
    {
    }

    void test_case_start(const TestCaseData& in) override
    {
        tc = &in;
        Log::info("[RUN     ] {}", in.m_name);
    }

    // called when a test case is reentered because of unfinished subcases
    void test_case_reenter(const TestCaseData& /*in*/) override
    {
    }

    void test_case_end(const CurrentTestCaseStats& in) override
    {
        if (in.failure_flags == TestCaseFailureReason::None) {
            Log::info("[    DONE] {}s", in.seconds);
        } else {
            Log::error("[    FAIL] {}s", in.seconds);
        }
    }

    void test_case_exception(const TestCaseException& /*in*/) override
    {
    }

    void subcase_start(const SubcaseSignature& in) override
    {
        Log::info("  [{}]", in.m_name);
    }

    void subcase_end() override
    {
    }

    void log_assert(const AssertData& in) override
    {
        // don't include successful asserts by default - this is done here
        // instead of in the framework itself because doctest doesn't know
        // if/when a reporter/listener cares about successful results
        if (!in.m_failed && !opt.success)
            return;

        // make sure there are no races - this is done here instead of in the
        // framework itself because doctest doesn't know if reporters/listeners
        // care about successful asserts and thus doesn't lock a mutex unnecessarily
        std::lock_guard<std::mutex> lock(mutex);

        // ...
    }

    void log_message(const MessageData& /*in*/) override
    {
        // messages too can be used in a multi-threaded context - like asserts
        std::lock_guard<std::mutex> lock(mutex);

        // ...
    }

    void test_case_skipped(const TestCaseData& /*in*/) override
    {
    }
};

REGISTER_LISTENER("my_listener", 1, TestReporter);
}
