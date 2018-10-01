#pragma once

#include <date/date.h>

#include <chrono>
#include <ostream>

namespace opaq {

namespace chrono {
using days      = date::days;
using date_time = std::chrono::system_clock::time_point;

template <typename T>
std::chrono::seconds to_seconds(const T& timePoint)
{
    return std::chrono::duration_cast<std::chrono::seconds>(timePoint);
}

template <typename T>
chrono::days to_days(const T& timePoint)
{
    return std::chrono::duration_cast<chrono::days>(timePoint);
}

// to YYYYMMDD format
std::string to_dense_date_string(const date_time& dt);
// to YYYY-MM-DD format
std::string to_date_string(const date_time& dt);
// to YYYY-MM-DD HH:MM:SS format
std::string to_string(const date_time& dt);

date_time from_date_string(std::string_view s);
date_time from_date_time_string(std::string_view s);

date_time make_date_time(date::year_month_day ymd);
date_time make_date_time(int year, int month, int day);
bool is_weekend(const date_time& dt);
}

namespace chrono_literals {
constexpr chrono::days operator""_d(unsigned long long int d)
{
    return chrono::days(d);
}
}

inline std::ostream& operator<<(std::ostream& os, const chrono::date_time& dt)
{
    os << chrono::to_string(dt);
    return os;
}
}
