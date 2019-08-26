#pragma once

#include <date/date.h>
#include <fmt/chrono.h>

namespace inf::chrono {

using days       = date::days;
using date_point = std::chrono::time_point<std::chrono::system_clock, days>;
using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;

inline date::year_month_day to_year_month_day(time_point tp)
{
    return date::year_month_day(date::sys_days(std::chrono::floor<date::days>(tp)));
}

/*! Converts a time point to string in the standard date and time string (locale dependant) */
template <typename Clock, typename Duration>
std::string to_string(std::chrono::time_point<Clock, Duration> tp)
{
    std::time_t time = Clock::to_time_t(tp);
    std::tm* tm      = std::localtime(&time);
    return fmt::format("{:%c}", *tm);
}

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
    std::tm* tm      = std::localtime(&time);
    return fmt::format(fmt::format("{{:{}}}", format), *tm);
}

template <typename Clock, typename Duration>
std::string to_utc_string(std::string_view format, std::chrono::time_point<Clock, Duration> tp)
{
    std::time_t time = Clock::to_time_t(tp);
    std::tm* tm      = std::gmtime(&time);
    return fmt::format(fmt::format("{{:{}}}", format), *tm);
}

class DurationRecorder
{
public:
    DurationRecorder()
    : _startTime(std::chrono::high_resolution_clock::now())
    {
    }

    std::chrono::seconds elapsed_seconds() const
    {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - _startTime);
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

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;
};

}
