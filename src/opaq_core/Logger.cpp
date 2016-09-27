#include "Logger.h"

#include <spdlog/details/log_msg.h>

std::unique_ptr<LogConfiguration> Log::_config;

#ifdef WIN32

class OutputDebugSink : public spdlog::sinks::sink
{
    void log(const spdlog::details::log_msg& msg) override
    {
        OutputDebugString(msg.formatted.c_str());
    }

    void flush()
    {
    }
};

#endif

void Log::initLogger(const std::string& filename)
{
    return filename.empty() ? initConsoleLogger() : initFileLogger(filename);
}

void Log::initConsoleLogger()
{
#ifdef WIN32
    auto sink = std::make_shared<OutputDebugSink>();
#else
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
    // enable color output on linux
    auto sink = std::make_shared<spdlog::sinks::ansicolor_sink>(consoleSink);
#endif

    _config = std::make_unique<LogConfiguration>();
    _config->sinks.push_back(sink);
    _config->pattern = "[%l] [%n] %v";
    _config->level = spdlog::level::info;
}

void Log::initFileLogger(const std::string& filename)
{
    _config = std::make_unique<LogConfiguration>();
    _config->sinks.push_back(std::make_shared<spdlog::sinks::simple_file_sink_mt>(filename, true /* truncate */));
    _config->pattern = "%+";
    _config->level = spdlog::level::info;
}

void Log::initLogger(const LogConfiguration& config)
{
    _config = std::make_unique<LogConfiguration>(config);
}

LogConfiguration& Log::getConfiguration()
{
    return *_config;
}

std::shared_ptr<spdlog::logger> Log::getLogger(const std::string& name)
{
    return spdlog::get(name);
}

std::shared_ptr<spdlog::logger> Log::createLogger(const std::string& name)
{
    auto logger = getLogger(name);
    if (!logger)
    {
        assert(!_config->sinks.empty());
        logger = std::make_shared<spdlog::logger>(name, begin(_config->sinks), end(_config->sinks));
        logger->set_pattern(_config->pattern);
        logger->set_level(_config->level);
        //register it if you need to access it globally
        spdlog::register_logger(logger);
    }

    return logger;
}
