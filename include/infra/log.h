#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace infra {

class Log
{
public:
    enum class Level
    {
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    static void initialize();
    static void initialize(const std::shared_ptr<spdlog::sinks::sink>& sink);

    static void setLevel(Level level);

    template <class... T>
    static void debug(T&&... arg)
    {
        if (_log) {
            _log->debug(std::forward<T>(arg)...);
        }
    }

    template <class... T>
    static void info(T&&... arg)
    {
        if (_log) {
            _log->info(std::forward<T>(arg)...);
        }
    }

    template <class... T>
    static void warn(T&&... arg)
    {
        if (_log) {
            _log->warn(std::forward<T>(arg)...);
        }
    }

    template <class... T>
    static void error(T&&... arg)
    {
        if (_log) {
            _log->error(std::forward<T>(arg)...);
        }
    }

    template <class... T>
    static void critical(T&&... arg)
    {
        if (_log) {
            _log->critical(std::forward<T>(arg)...);
        }
    }

private:
    static void initializeConsoleLogger();

    static std::shared_ptr<spdlog::logger> _log;
};
}
