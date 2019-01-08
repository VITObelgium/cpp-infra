#include "pythonlogsink.h"

namespace gdx::python {

namespace py = pybind11;

template <typename Mutex>
LogSink<Mutex>::LogSink()
{
    auto logging = py::module::import("logging");
    _logger      = logging.attr("getLogger")("gdx");

    _warnings = py::module::import("warnings");
}

template <typename Mutex>
void LogSink<Mutex>::sink_it_(const spdlog::details::log_msg& msg)
{
    fmt::memory_buffer formatted;
    spdlog::sinks::sink::formatter_->format(msg, formatted);

    switch (msg.level) {
    case spdlog::level::info:
        _logger.attr("info")(fmt::to_string(formatted).c_str());
        break;
    case spdlog::level::warn:
        //_logger.attr("warning")(fmt::to_string(formatted).c_str());
        showWarning(fmt::to_string(msg.raw).c_str());
        break;
    case spdlog::level::err:
        _logger.attr("error")(fmt::to_string(formatted).c_str());
        break;
    case spdlog::level::critical:
        _logger.attr("critical")(fmt::to_string(formatted).c_str());
        break;
    case spdlog::level::debug:
    case spdlog::level::trace:
        _logger.attr("debug")(fmt::to_string(formatted).c_str());
        break;
    default:
        break;
    }
}

template <typename Mutex>
void LogSink<Mutex>::flush_()
{
}

template <typename Mutex>
void LogSink<Mutex>::showWarning(py::str message)
{
#if PY_MAJOR_VERSION == 2
    auto builtin = py::module::import("__builtin__");
#else
    auto builtin = py::module::import("builtins");
#endif
    _warnings.attr("warn_explicit")(message, builtin.attr("UserWarning"), "gdx", 0);
}

template class LogSink<std::mutex>;
template class LogSink<spdlog::details::null_mutex>;

}
