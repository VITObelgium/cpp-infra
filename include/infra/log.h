#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace infra {

/*!
 * class for strongly typed log source name
 */
class LogSource
{
public:
    /*!
    * The logsource assumes the name of the log source is defined in static storage
    * that outlives the log source instance so we use a string_view
    */
    LogSource(std::string_view src) noexcept
    : _src(src)
    {
    }

    operator std::string_view() const noexcept
    {
        return _src;
    }

private:
    std::string_view _src;
};

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
    static void debug(const char* format, T&&... arg)
    {
        if (_log) {
            _log->debug(format, std::forward<T>(arg)...);
        }
    }

    static void debug(const std::string& msg)
    {
        if (_log) {
            _log->debug(msg);
        }
    }

    static void debug(const LogSource& src, std::string_view message)
    {
        if (_log) {
            _log->debug("[{}] {}", static_cast<std::string_view>(src), message);
        }
    }

    template <class... T>
    static void debug(const LogSource& src, const char* format, T&&... arg)
    {
        if (_log) {
            _log->debug("[{}] {}", static_cast<std::string_view>(src), fmt::format(format, std::forward<T>(arg)...));
        }
    }

    template <class... T>
    static void info(const char* format, T&&... arg)
    {
        if (_log) {
            _log->info(format, std::forward<T>(arg)...);
        }
    }

    static void info(const std::string& msg)
    {
        if (_log) {
            _log->info(msg);
        }
    }

    static void info(const LogSource& src, std::string_view message)
    {
        if (_log) {
            _log->info("[{}] {}", static_cast<std::string_view>(src), message);
        }
    }

    template <class... T>
    static void info(const LogSource& src, const char* format, T&&... arg)
    {
        if (_log) {
            _log->info("[{}] {}", static_cast<std::string_view>(src), fmt::format(format, std::forward<T>(arg)...));
        }
    }

    template <class... T>
    static void warn(const char* format, T&&... arg)
    {
        if (_log) {
            _log->warn(format, std::forward<T>(arg)...);
        }
    }

    static void warn(const std::string& msg)
    {
        if (_log) {
            _log->warn(msg);
        }
    }

    static void warn(const LogSource& src, std::string_view message)
    {
        if (_log) {
            _log->warn("[{}] {}", static_cast<std::string_view>(src), message);
        }
    }

    template <class... T>
    static void warn(const LogSource& src, const char* format, T&&... arg)
    {
        if (_log) {
            _log->warn("[{}] {}", static_cast<std::string_view>(src), fmt::format(format, std::forward<T>(arg)...));
        }
    }

    template <class... T>
    static void error(const char* format, T&&... arg)
    {
        if (_log) {
            _log->error(format, std::forward<T>(arg)...);
        }
    }

    static void error(const std::string& msg)
    {
        if (_log) {
            _log->error(msg);
        }
    }

    static void error(const LogSource& src, std::string_view message)
    {
        if (_log) {
            _log->error("[{}] {}", static_cast<std::string_view>(src), message);
        }
    }

    template <class... T>
    static void error(const LogSource& src, const char* format, T&&... arg)
    {
        if (_log) {
            _log->error("[{}] {}", static_cast<std::string_view>(src), fmt::format(format, std::forward<T>(arg)...));
        }
    }

    template <class... T>
    static void critical(const char* format, T&&... arg)
    {
        if (_log) {
            _log->critical(format, std::forward<T>(arg)...);
        }
    }

    static void critical(const std::string& msg)
    {
        if (_log) {
            _log->critical(msg);
        }
    }

    static void critical(const LogSource& src, std::string_view message)
    {
        if (_log) {
            _log->critical("[{}] {}", static_cast<std::string_view>(src), message);
        }
    }

    template <class... T>
    static void critical(const LogSource& src, const char* format, T&&... arg)
    {
        if (_log) {
            _log->critical("[{}] {}", static_cast<std::string_view>(src), fmt::format(format, std::forward<T>(arg)...));
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
