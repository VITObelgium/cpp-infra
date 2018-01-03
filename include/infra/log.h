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

    enum class Colored
    {
        On,
        Off
    };

    static void initializeConsoleOnly(const std::string& name, Colored colored);
    static void initialize(const std::string& name);
    static void uninitialize();

    static void addFileSink(const std::string& filePath);
    static void addConsoleSink(Colored option);
    static void addCustomSink(const spdlog::sink_ptr& sink);

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
    static std::shared_ptr<spdlog::logger> _log;
    static std::vector<spdlog::sink_ptr> _sinks;
};

struct LogRegistration
{
    LogRegistration(const std::string& name)
    {
        Log::initialize(name);
    }

    ~LogRegistration()
    {
        Log::uninitialize();
    }
};
}
