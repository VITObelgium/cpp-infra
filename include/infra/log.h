#pragma once

#include "infra/filesystem.h"

#include <memory>
#include <spdlog/spdlog.h>

namespace inf {

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
        Off,
        Trace,
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

    static void initialize_console_only(const std::string& name, Colored colored);
    static void initialize(const std::string& name);
    static void uninitialize();

    static void add_file_sink(const fs::path& filePath);
    static void add_rotating_file_sink(const fs::path& filePath, std::size_t maxFileSize);
    static void add_console_sink(Colored option);
    static void add_custom_sink(const spdlog::sink_ptr& sink);

    static void set_level(Level level);
    static void set_pattern(std::string_view pattern);

    template <class... T>
    static void log(Level level, const char* format, T&&... args)
    {
        if (_log) {
            switch (level) {
            case Level::Trace:
                _log->trace(fmt::runtime(format), std::forward<T>(args)...);
                break;
            case Level::Debug:
                _log->debug(fmt::runtime(format), std::forward<T>(args)...);
                break;
            case Level::Info:
                _log->info(fmt::runtime(format), std::forward<T>(args)...);
                break;
            case Level::Warning:
                _log->warn(fmt::runtime(format), std::forward<T>(args)...);
                break;
            case Level::Error:
                _log->error(fmt::runtime(format), std::forward<T>(args)...);
                break;
            case Level::Critical:
                _log->critical(fmt::runtime(format), std::forward<T>(args)...);
                break;
            default:
                break;
            }
        }
    }

    template <class... T>
    static void trace(const char* format, T&&... args)
    {
        if (_log) {
            _log->trace(fmt::runtime(format), std::forward<T>(args)...);
        }
    }

    static void trace(const std::string& msg)
    {
        if (_log) {
            _log->trace(msg);
        }
    }

    static void trace(const LogSource& src, std::string_view message)
    {
        if (_log) {
            _log->trace("[{}] {}", static_cast<std::string_view>(src), message);
        }
    }

    template <class... T>
    static void trace(const LogSource& src, const char* format, T&&... args)
    {
        if (_log) {
            _log->trace("[{}] {}", static_cast<std::string_view>(src), fmt::format(format, std::forward<T>(args)...));
        }
    }

    template <class... T>
    static void debug(const char* format, T&&... args)
    {
        if (_log) {
            _log->debug(fmt::runtime(format), std::forward<T>(args)...);
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
    static void debug(const LogSource& src, const char* format, T&&... args)
    {
        if (_log) {
            _log->debug("[{}] {}", static_cast<std::string_view>(src), fmt::format(format, std::forward<T>(args)...));
        }
    }

    template <class... T>
    static void info(const char* format, T&&... args)
    {
        if (_log) {
            _log->info(fmt::runtime(format), std::forward<T>(args)...);
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
    static void info(const LogSource& src, const char* format, T&&... args)
    {
        if (_log) {
            _log->info("[{}] {}", static_cast<std::string_view>(src), fmt::format(format, std::forward<T>(args)...));
        }
    }

    template <class... T>
    static void warn(const char* format, T&&... args)
    {
        if (_log) {
            _log->warn(fmt::runtime(format), std::forward<T>(args)...);
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
    static void warn(const LogSource& src, const char* format, T&&... args)
    {
        if (_log) {
            _log->warn("[{}] {}", static_cast<std::string_view>(src), fmt::format(format, std::forward<T>(args)...));
        }
    }

    template <class... T>
    static void error(const char* format, T&&... args)
    {
        if (_log) {
            _log->error(fmt::runtime(format), std::forward<T>(args)...);
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
    static void error(const LogSource& src, const char* format, T&&... args)
    {
        if (_log) {
            _log->error("[{}] {}", static_cast<std::string_view>(src), fmt::format(format, std::forward<T>(args)...));
        }
    }

    template <class... T>
    static void critical(const char* format, T&&... args)
    {
        if (_log) {
            _log->critical(fmt::runtime(format), std::forward<T>(args)...);
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
    static void critical(const LogSource& src, const char* format, T&&... args)
    {
        if (_log) {
            _log->critical("[{}] {}", static_cast<std::string_view>(src), fmt::format(format, std::forward<T>(args)...));
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
