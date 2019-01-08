#pragma once

#include <mutex>
#include <pybind11/pybind11.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

namespace gdx::python {

template <typename Mutex>
class LogSink : public spdlog::sinks::base_sink<Mutex>
{
public:
    LogSink();

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override;

private:
    void showWarning(pybind11::str msg);

    pybind11::object _logger;
    pybind11::object _warnings;
};

using LogSinkMt = LogSink<std::mutex>;
using LogSinkSt = LogSink<spdlog::details::null_mutex>;
}
