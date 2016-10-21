#include "DateTime.h"
#include "Exceptions.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace OPAQ
{
namespace chrono
{
    
std::string to_date_string(const date_time& dt)
{
    std::time_t t = std::chrono::system_clock::to_time_t(dt);
    std::tm tm = *std::gmtime(&t);

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d");
    return ss.str();
}

std::string to_string(const date_time& dt)
{
    std::time_t t = std::chrono::system_clock::to_time_t(dt);
    std::tm tm = *std::gmtime(&t);

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

date_time from_date_string(const std::string& s)
{
    std::stringstream input(s);
    input.imbue(std::locale(setlocale(LC_ALL, nullptr)));

    std::tm tm{ 0 };
    input >> std::get_time(&tm, "%Y-%m-%d");
    if (input.fail())
    {
        throw InvalidArgumentsException("Could not parse date: {}", s);
    }

    return make_date_time(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

date_time from_date_time_string(const std::string& s)
{
    std::stringstream input(s);
    input.imbue(std::locale(setlocale(LC_ALL, nullptr)));

    std::tm tm{ 0 };
    input >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (input.fail())
    {
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
    auto weekDay = date::weekday(date::sys_days(std::chrono::floor<days>(dt)));
    return weekDay == date::sun || weekDay == date::sat;
}

}
}