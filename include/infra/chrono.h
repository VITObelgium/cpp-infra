#pragma once

#include <date/date.h>
#include <fmt/time.h>

namespace inf::chrono {

using days       = date::days;
using date_point = std::chrono::time_point<std::chrono::system_clock, days>;
using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;

/*! Converts a time point to string in the standard date and time string (locale dependant) */
template <typename Clock, typename Duration>
std::string to_string(std::chrono::time_point<Clock, Duration> tp)
{
    std::time_t time = typename Clock::to_time_t(tp);
    std::tm* tm      = std::localtime(&time);
    return fmt::format("{:%c}", *tm);
}

/*! Converts a time point to string using the provided format specification
 * e.g.: "%Y-%m-%d"
 * see https://en.cppreference.com/w/cpp/chrono/c/strftime for full format specification
 */
template <typename Clock, typename Duration>
std::string to_string(std::string_view format, std::chrono::time_point<Clock, Duration> tp)
{
    std::time_t time = typename Clock::to_time_t(tp);
    std::tm* tm      = std::localtime(&time);
    return fmt::format(fmt::format("{{:{}}}", format), *tm);
}

}
