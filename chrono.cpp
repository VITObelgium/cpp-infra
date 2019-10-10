#include "infra/chrono.h"
#include "infra/exception.h"

#include <sstream>

namespace inf::chrono {

using namespace date;

time_point system_time_point_from_string(std::string_view str1, const char* format)
{
    time_point tp;
    std::istringstream ss;
    ss.str(std::string(str1));
    ss >> date::parse(format, tp);
    return tp;
}

}
