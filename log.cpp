#include "infra/log.h"

#include <spdlog/details/log_msg.h>

namespace infra {

std::shared_ptr<spdlog::logger> Log::_log;
std::vector<spdlog::sink_ptr> Log::_sinks;

void Log::addFileSink(const std::string& filePath)
{
    if (_log) {
        throw std::runtime_error("Sinks need to be added before initialising the logging system");
    }

    _sinks.push_back(std::make_shared<spdlog::sinks::simple_file_sink_mt>(filePath, true));
}

void Log::addConsoleSink(Colored colored)
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
}

void Log::addCustomSink(const spdlog::sink_ptr& sink)
{
    if (_log) {
        throw std::runtime_error("Sinks need to be added before initialising the logging system");
    }

    _sinks.push_back(sink);
}

void Log::initializeConsoleOnly(const std::string& name, Colored colored)
{
    addConsoleSink(colored);
    initialize(name);
}

void Log::initialize(const std::string& name)
{
    if (_sinks.empty()) {
        throw std::runtime_error("No sinks added before initializing the logging system");
    }

    _log = std::make_shared<spdlog::logger>(name, begin(_sinks), end(_sinks));
    _log->set_pattern("[%l] %v");
    _log->set_level(spdlog::level::warn);
}

void Log::uninitialize()
{
    _sinks.clear();
    spdlog::drop_all();
    _log.reset();
}

void Log::setLevel(Level level)
{
    if (!_log) {
        throw std::runtime_error("Initialise the logging system before setting the log level");
    }

    auto spdLevel = spdlog::level::off;

    switch (level) {
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
}
