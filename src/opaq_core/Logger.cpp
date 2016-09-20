#include "Logger.h"

std::shared_ptr<spdlog::sinks::sink> Log::_sink;

void Log::initLogger(const std::string& filename)
{
    return filename.empty() ? initConsoleLogger() : initFileLogger(filename);
}

void Log::initConsoleLogger()
{
    _sink = std::make_shared<spdlog::sinks::stdout_sink_st>();
    //layout->setConversionPattern("%m%n");
}

void Log::initFileLogger(const std::string& filename)
{
    _sink = std::make_shared<spdlog::sinks::simple_file_sink_st>(filename, true /* truncate */);
    //layout->setConversionPattern("%d %-5p (%c) %m%n");
}

std::shared_ptr<spdlog::logger> Log::getLogger(const std::string& name)
{
    return spdlog::get(name);
}

std::shared_ptr<spdlog::logger> Log::createLogger(const std::string& name)
{
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(_sink);
    auto logger = std::make_shared<spdlog::logger>(name, begin(sinks), end(sinks));
    //register it if you need to access it globally
    spdlog::register_logger(logger);

    return logger;
}
