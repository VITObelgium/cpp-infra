#include "infra/chrono.h"
#include "infra/exception.h"

#include <sstream>

namespace inf::chrono {

#if __cplusplus > 201703L
#include <format>

namespace date = std::chrono;
#else
using namespace date;
#endif

date_point today()
{
    return date::year_month_day(date::sys_days(std::chrono::floor<date::days>(std::chrono::system_clock::now())));
}

local_date_point today_local()
{
    const auto now = date::zoned_time(date::current_zone(), std::chrono::system_clock::now());
    return std::chrono::floor<date::days>(now.get_local_time());
}

time_point now()
{
    return std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
}

local_time_point now_local()
{
    const auto now = date::zoned_time(date::current_zone(), std::chrono::system_clock::now());
    return std::chrono::floor<std::chrono::milliseconds>(now.get_local_time());
}

date_point date_from_time_point(time_point tp)
{
    return std::chrono::floor<date::days>(tp);
}

local_date_point date_from_time_point(local_time_point tp)
{
    return std::chrono::floor<date::days>(tp);
}

#if __cplusplus > 201703L
std::chrono::hh_mm_ss<std::chrono::milliseconds> time_of_day(time_point tp)
{
    auto dp = std::chrono::floor<std::chrono::days>(tp);
    return std::chrono::hh_mm_ss(tp - dp); // Yields time_of_day type
}

std::chrono::year_month_day to_year_month_day(time_point tp)
{
    return std::chrono::year_month_day(std::chrono::sys_days(std::chrono::floor<std::chrono::days>(tp)));
}

std::chrono::year_month_day to_year_month_day(local_time_point tp)
{
    return std::chrono::year_month_day(std::chrono::local_days(std::chrono::floor<std::chrono::days>(tp)));
}

std::string to_string(std::chrono::local_seconds tp)
{
    return to_string("%Y_%m_%d_%H.%M", tp);
}

std::string to_string(std::string_view format, std::chrono::local_seconds tp)
{
    std::ostringstream dateStr;
    dateStr << std::vformat(std::format("{{:{}}}", format), std::make_format_args(tp));
    return dateStr.str();
}

std::string to_string(local_time_point tp)
{
    std::stringstream ss;
    ss << tp;
    return ss.str();
}
#else
date::hh_mm_ss<std::chrono::milliseconds> time_of_day(time_point tp)
{
    auto dp = date::floor<date::days>(tp);
    return date::make_time(tp - dp); // Yields time_of_day type
}

date::year_month_day to_year_month_day(time_point tp)
{
    return date::year_month_day(date::sys_days(std::chrono::floor<date::days>(tp)));
}

date::year_month_day to_year_month_day(local_time_point tp)
{
    return date::year_month_day(date::local_days(std::chrono::floor<date::days>(tp)));
}

std::string to_string(date::local_seconds tp)
{
    return to_string("%Y_%m_%d_%H.%M", tp);
}

std::string to_string(std::string_view format, date::local_seconds tp)
{
    std::ostringstream dateStr;
    dateStr << date::format(std::string(format).c_str(), tp);
    return dateStr.str();
}

std::string to_string(local_time_point tp)
{
    std::stringstream ss;
    ss << tp;
    return ss.str();
}
#endif

template <typename TimeType>
std::optional<TimeType> time_point_from_string(std::string_view str1, const char* format)
{
    TimeType tp;
    std::istringstream ss;
    ss.str(std::string(str1));
    ss >> date::parse(format, tp);
    if (ss.fail()) {
        return {};
    }

    return tp;
}

std::optional<time_point> system_time_point_from_string(std::string_view str1, const char* format)
{
    return time_point_from_string<time_point>(str1, format);
}

std::optional<local_time_point> local_time_point_from_string(std::string_view str1, const char* format)
{
    return time_point_from_string<local_time_point>(str1, format);
}

std::optional<time_point> localtime_to_utc(time_point dt, date::choose* choice)
{
    auto ymd = chrono::to_year_month_day(dt);
    auto tod = chrono::time_of_day(dt); // Yields time_of_day type

    auto tp = date::local_days{ymd} + tod.hours() + tod.minutes() + tod.seconds();
    auto z  = date::current_zone();

    std::optional<time_point> utcTime;
    auto i = z->get_info(tp);
    switch (i.result) {
    case date::local_info::unique: {
        date::zoned_time<std::chrono::seconds> zt(z, tp); //"Europe/Brussels"
        utcTime = std::optional<time_point>(zt.get_sys_time());
        break;
    }
    case date::local_info::ambiguous: {
        if (choice) {
            date::zoned_time<std::chrono::seconds> zt(z, tp, *choice);
            utcTime = std::optional<time_point>(zt.get_sys_time());
        }
        break;
    }
    case date::local_info::nonexistent:
        break;
    default:
        break;
    }

    return utcTime;
}

}
