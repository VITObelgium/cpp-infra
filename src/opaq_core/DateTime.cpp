#include "DateTime.h"
#include "Exceptions.h"

#include <ctime>
#include <iomanip>
#include <sstream>

#include <fmt/format.h>

namespace opaq {
namespace chrono {

std::string to_dense_date_string(const date_time& dt)
{
    const auto ymd = date::year_month_day(date::floor<date::days>(dt));
    return fmt::format("{}{:0=2}{:0=2}", static_cast<int>(ymd.year()), static_cast<unsigned>(ymd.month()), static_cast<unsigned>(ymd.day()));
}

std::string to_date_string(const date_time& dt)
{
    const auto ymd = date::year_month_day(date::floor<date::days>(dt));
    return fmt::format("{}-{:0=2}-{:0=2}", static_cast<int>(ymd.year()), static_cast<unsigned>(ymd.month()), static_cast<unsigned>(ymd.day()));
}

std::string to_string(const date_time& dt)
{
    const auto days = date::floor<date::days>(dt);
    const auto ymd  = date::year_month_day(days);
    const auto time = date::make_time(dt - days).make24();

    return fmt::format("{}-{:0=2}-{:0=2} {:0=2}:{:0=2}:{:0=2}",
        static_cast<int>(ymd.year()),
        static_cast<unsigned>(ymd.month()),
        static_cast<unsigned>(ymd.day()),
        time.hours().count(),
        time.minutes().count(),
        time.seconds().count());
}

date_time from_date_string(std::string_view s)
{
    std::stringstream input;
    input.write(s.data(), s.size());

    std::tm tm{0};
    input >> std::get_time(&tm, "%Y-%m-%d");
    if (input.fail()) {
        throw InvalidArgumentsException("Could not parse date: {}", s);
    }

    return make_date_time(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

date_time from_date_time_string(std::string_view s)
{
    std::stringstream input;
    input.write(s.data(), s.size());

    std::tm tm{0};
    input >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (input.fail()) {
        throw InvalidArgumentsException("Could not parse date time: {}", s);
    }

    auto date = make_date_time(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    date += std::chrono::hours(tm.tm_hour);
    date += std::chrono::minutes(tm.tm_min);
    date += std::chrono::seconds(tm.tm_sec);

    return date;
}

date_time make_date_time(date::year_month_day ymd)
{
    return date::sys_days(ymd);
}

date_time make_date_time(int year, int month, int day)
{
    return make_date_time(date::year_month_day(date::year(year), date::month(month), date::day(day)));
}

bool is_weekend(const date_time& dt)
{
    auto weekDay = date::weekday(date::sys_days(date::floor<days>(dt)));
    return weekDay == date::sun || weekDay == date::sat;
}
}
}
