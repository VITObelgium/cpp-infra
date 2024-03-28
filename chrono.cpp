#include "infra/chrono.h"

#include <sstream>

#include <fmt/chrono.h>

namespace inf::chrono {

date_point today()
{
#ifdef HAVE_CPP20_CHRONO
    return std::chrono::year_month_day(std::chrono::sys_days(std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())));
#else
    return date::year_month_day(date::sys_days(std::chrono::floor<date::days>(std::chrono::system_clock::now())));
#endif
}

local_date_point today_local()
{
#ifdef HAVE_CPP20_CHRONO
    const auto now = std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now());
    return std::chrono::floor<std::chrono::days>(now.get_local_time());
#else
    const auto now = date::zoned_time(date::current_zone(), std::chrono::system_clock::now());
    return std::chrono::floor<date::days>(now.get_local_time());
#endif
}

time_point now()
{
    return std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
}

local_time_point now_local()
{
#ifdef HAVE_CPP20_CHRONO
    const auto now = std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now());
    return std::chrono::floor<std::chrono::milliseconds>(now.get_local_time());
#else
    const auto now = date::zoned_time(date::current_zone(), std::chrono::system_clock::now());
    return std::chrono::floor<std::chrono::milliseconds>(now.get_local_time());
#endif
}

date_point date_from_time_point(time_point tp)
{
#ifdef HAVE_CPP20_CHRONO
    return std::chrono::floor<std::chrono::days>(tp);
#else
    return std::chrono::floor<date::days>(tp);
#endif
}

local_date_point date_from_time_point(local_time_point tp)
{
#ifdef HAVE_CPP20_CHRONO
    return std::chrono::floor<std::chrono::days>(tp);
#else
    return std::chrono::floor<date::days>(tp);
#endif
}

#ifdef HAVE_CPP20_CHRONO
std::chrono::hh_mm_ss<std::chrono::milliseconds> time_of_day(time_point tp)
{
    auto dp = std::chrono::floor<std::chrono::days>(tp);
    return std::chrono::hh_mm_ss(tp - dp); // Yields time_of_day type
}

std::chrono::hh_mm_ss<std::chrono::milliseconds> time_of_day(local_time_point tp)
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
    return fmt::format(fmt::runtime(fmt::format("{{:{}}}", format)), tp);
}

std::string to_string(local_time_point tp)
{
    return fmt::format("{:%F}", tp);
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

#if defined(HAVE_CPP20_CHRONO) && !defined(NO_CHRONO_PARSE_SUPPORT)
    std::istringstream ss;
    ss.str(std::string(str1));
    ss >> std::chrono::parse(format, tp);
    if (ss.fail()) {
        return {};
    }
#else
    std::istringstream ss;
    ss.str(std::string(str1));
    ss >> date::parse(format, tp);
    if (ss.fail()) {
        return {};
    }
#endif

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

#ifdef HAVE_CPP20_CHRONO
std::optional<time_point> localtime_to_utc(time_point dt, std::chrono::choose* choice)
{
    auto ymd = chrono::to_year_month_day(dt);
    auto tod = chrono::time_of_day(dt); // Yields time_of_day type

    auto tp = std::chrono::local_days{ymd} + tod.hours() + tod.minutes() + tod.seconds();
    auto z  = std::chrono::current_zone();

    std::optional<time_point> utcTime;
    auto i = z->get_info(tp);
    switch (i.result) {
    case std::chrono::local_info::unique: {
        std::chrono::zoned_time<std::chrono::seconds> zt(z, tp); //"Europe/Brussels"
        utcTime = std::optional<time_point>(zt.get_sys_time());
        break;
    }
    case std::chrono::local_info::ambiguous: {
        if (choice) {
            std::chrono::zoned_time<std::chrono::seconds> zt(z, tp, *choice);
            utcTime = std::optional<time_point>(zt.get_sys_time());
        }
        break;
    }
    case std::chrono::local_info::nonexistent:
        break;
    default:
        break;
    }

    return utcTime;
}
#else
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
#endif

}
