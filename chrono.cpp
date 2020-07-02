#include "infra/chrono.h"
#include "infra/exception.h"

#include <sstream>

namespace inf::chrono {

using namespace date;

date::year_month_day to_year_month_day(time_point tp)
{
    return date::year_month_day(date::sys_days(std::chrono::floor<date::days>(tp)));
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

std::optional<time_point> system_time_point_from_string(std::string_view str1, const char* format)
{
    time_point tp;
    std::istringstream ss;
    ss.str(std::string(str1));
    ss >> date::parse(format, tp);
    if (ss.fail()) {
        return {};
    }

    return tp;
}

}
