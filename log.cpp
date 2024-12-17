#include "infra/log.h"
#include "infra/exception.h"
#include "infra/string.h"

#include <spdlog/async.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

#ifdef _WIN32
#include <spdlog/sinks/wincolor_sink.h>
#endif

namespace inf {

std::shared_ptr<spdlog::logger> Log::_log;
std::vector<spdlog::sink_ptr> Log::_sinks;

void Log::add_file_sink(const fs::path& filePath)
{
    if (_log) {
        throw std::runtime_error("Sinks need to be added before initialising the logging system");
    }

    _sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(str::from_u8(filePath.u8string()), true));
}

void Log::add_rotating_file_sink(const fs::path& filePath, std::size_t maxFileSize)
{
    if (_log) {
        throw std::runtime_error("Sinks need to be added before initialising the logging system");
    }

    constexpr std::size_t maxFileCount = 5;
    _sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(str::from_u8(filePath.u8string()), maxFileSize, maxFileCount));
}

void Log::add_console_sink(Colored colored)
{
    if (_log) {
        throw std::runtime_error("Sinks need to be added before initialising the logging system");
    }

    if (colored == Colored::On) {
#ifdef _WIN32
        _sinks.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>());
#else
        _sinks.push_back(std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>());
#endif
    } else {
        _sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
    }

#ifdef _WIN32
    _sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
#endif
}

void Log::add_custom_sink(const spdlog::sink_ptr& sink)
{
    if (_log) {
        throw std::runtime_error("Sinks need to be added before initialising the logging system");
    }

    _sinks.push_back(sink);
}

void Log::initialize_console_only(const std::string& name, Colored colored)
{
    add_console_sink(colored);
    initialize(name);
}

void Log::initialize(const std::string& name)
{
    if (_sinks.empty()) {
        throw std::runtime_error("No sinks added before initializing the logging system");
    }

    _log = std::make_shared<spdlog::logger>(name, begin(_sinks), end(_sinks));
    _log->set_pattern("%^[%L] %v%$");
    _log->set_level(spdlog::level::warn);
}

void Log::initialize_async(const std::string& name)
{
    if (_sinks.empty()) {
        throw std::runtime_error("No sinks added before initializing the logging system");
    }

    spdlog::init_thread_pool(8192, 1);

    _log = std::make_shared<spdlog::async_logger>(name, begin(_sinks), end(_sinks), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    _log->set_pattern("%^[%L] %v%$");
    _log->set_level(spdlog::level::warn);
}

void Log::uninitialize()
{
    _sinks.clear();
    spdlog::drop_all();
    _log.reset();
}

void Log::set_level(Level level)
{
    if (!_log) {
        throw std::runtime_error("Initialise the logging system before setting the log level");
    }

    auto spdLevel = spdlog::level::off;

    switch (level) {
    case Level::Off:
        break;
    case Level::Trace:
        spdLevel = spdlog::level::trace;
        break;
    case Level::Debug:
        spdLevel = spdlog::level::debug;
        break;
    case Level::Info:
        spdLevel = spdlog::level::info;
        break;
    case Level::Warning:
        spdLevel = spdlog::level::warn;
        break;
    case Level::Error:
        spdLevel = spdlog::level::err;
        break;
    case Level::Critical:
        spdLevel = spdlog::level::critical;
        break;
    }

    _log->set_level(spdLevel);
}

void Log::set_pattern(std::string_view pattern)
{
    _log->set_pattern(fmt::format("%^{}%$", pattern));
}

Log::Level log_level_from_value(int32_t value)
{
    switch (value) {
    case 1:
        return Log::Level::Debug;
    case 2:
        return Log::Level::Info;
    case 3:
        return Log::Level::Warning;
    case 4:
        return Log::Level::Error;
    case 5:
        return Log::Level::Critical;
    default:
        throw RuntimeError("Invalid log level specified '{}': value must be in range [1-5]", value);
    }
}
}
