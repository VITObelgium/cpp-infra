#pragma once

#include <date/date.h>
#include <date/tz.h>
#include <fmt/chrono.h>
#include <optional>

#ifdef INFRA_LOG_ENABLED
#include "infra/log.h"
#endif

namespace inf::chrono {

using days       = date::days;
using date_point = std::chrono::time_point<std::chrono::system_clock, days>;
using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;

using local_date_point = date::local_days;
using local_time_point = date::local_time<std::chrono::milliseconds>;

date_point today();
local_date_point today_local();
time_point now();
local_time_point now_local();

date_point date_from_time_point(time_point tp);
local_date_point date_from_time_point(local_time_point tp);

date::hh_mm_ss<std::chrono::milliseconds> time_of_day(time_point tp);
date::year_month_day to_year_month_day(time_point tp);
date::year_month_day to_year_month_day(local_time_point tp);

/*! Converts a year_month_day to a system timepoint (locale dependent) */
inline time_point to_system_time_point(date::year_month_day ymd)
{
    return static_cast<date::sys_days>(ymd);
}

std::string to_string(date::local_seconds tp);
std::string to_string(std::string_view format, date::local_seconds tp);

/*! Converts a time point to string in the standard date and time string (locale dependent) */
template <typename Clock, typename Duration>
std::string to_string(std::chrono::time_point<Clock, Duration> tp)
{
    std::time_t time = Clock::to_time_t(tp);
    return fmt::format("{:%c}", fmt::localtime(time));
}

std::string to_string(local_time_point tp);

/*! Converts a time point to string using the provided format specification
 * e.g.: "%Y-%m-%d"
 * see https://en.cppreference.com/w/cpp/chrono/c/strftime for full format specification
 * Uses the current locale and timezone so different inputs can produce the same value in case
 * of daylight savings adjustments
 */
template <typename Clock, typename Duration>
std::string to_string(std::string_view format, std::chrono::time_point<Clock, Duration> tp)
{
    std::time_t time = Clock::to_time_t(tp);
    return fmt::format(fmt::format("{{:{}}}", format), fmt::localtime(time));
}

/*! Converts a string to a time point using the provided format specification
 * e.g.: "%Y-%m-%d"
 * see https://en.cppreference.com/w/cpp/chrono/c/strftime for full format specification
 * Uses the current locale and timezone so different inputs can produce the same value in case
 * of daylight savings adjustments
 */
std::optional<time_point> system_time_point_from_string(std::string_view str, const char* format);
std::optional<local_time_point> local_time_point_from_string(std::string_view str, const char* format);

template <typename Clock, typename Duration>
std::string to_utc_string(std::chrono::time_point<Clock, Duration> tp)
{
    std::time_t time = Clock::to_time_t(tp);
    return fmt::format("{:%c}", fmt::gmtime(time));
}

template <typename Clock, typename Duration>
std::string to_utc_string(std::string_view format, std::chrono::time_point<Clock, Duration> tp)
{
    std::time_t time = Clock::to_time_t(tp);
    return fmt::format(fmt::runtime(fmt::format("{{:{}}}", format)), fmt::gmtime(time));
}

std::optional<time_point> localtime_to_utc(time_point dt, date::choose* choice = nullptr);

class DurationRecorder
{
public:
    DurationRecorder()
    : _startTime(std::chrono::high_resolution_clock::now())
    {
    }

    std::chrono::seconds elapsed_seconds() const
    {
        return elapsed_time<std::chrono::seconds>();
    }

    template <typename Duration = std::chrono::high_resolution_clock::duration>
    Duration elapsed_time() const
    {
        const auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<Duration>(now - _startTime);
    }

    std::string elapsed_time_string() const
    {
        using namespace std::chrono_literals;

        auto duration = std::chrono::high_resolution_clock::now() - _startTime;

        if (duration > 60s) {
            auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration - minutes);
            return fmt::format("{} minutes {} seconds", minutes.count(), seconds.count());
        } else {
            auto seconds      = std::chrono::duration_cast<std::chrono::seconds>(duration);
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration - seconds);
            return fmt::format("{}.{:03d} seconds", seconds.count(), milliseconds.count());
        }
    }

    void reset()
    {
        _startTime = std::chrono::high_resolution_clock::now();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;
};

#ifdef INFRA_LOG_ENABLED
class ScopedDurationLog
{
public:
    ScopedDurationLog(std::string_view name, Log::Level level = Log::Level::Debug)
    : _name(name)
    , _level(level)
    {
        Log::log(_level, "{}", _name);
    }

    ~ScopedDurationLog() noexcept
    {
        Log::log(_level, "{} took {}", _name, _duration.elapsed_time_string());
    }

private:
    std::string _name;
    Log::Level _level;
    DurationRecorder _duration;
};
#endif

}
