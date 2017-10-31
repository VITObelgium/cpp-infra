#include "infra/log.h"

#include <spdlog/details/log_msg.h>

namespace infra {

std::shared_ptr<spdlog::logger> Log::_log;

void Log::initialize()
{
    initializeConsoleLogger();
}

void Log::initialize(const std::shared_ptr<spdlog::sinks::sink>& sink)
{
    _log = std::make_shared<spdlog::logger>("gdx", sink);
    _log->set_pattern("[%l] %v");
    _log->set_level(spdlog::level::warn);
}

void Log::initializeConsoleLogger()
{
    _log = spdlog::stdout_color_st("gdx");
    _log->set_pattern("[%l] %v");
    _log->set_level(spdlog::level::warn);
}

void Log::setLevel(Level level)
{
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
