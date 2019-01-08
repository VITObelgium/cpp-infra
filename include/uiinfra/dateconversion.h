#pragma once

#include <date/date.h>
#include <qdatetime.h>

namespace uiinfra {

template <class Clock, class Duration>
QDate toDate(const std::chrono::time_point<Clock, Duration>& tp)
{
    date::year_month_day date(std::chrono::floor<date::days>(tp));
    return QDate(int(date.year()), static_cast<unsigned int>(date.month()), static_cast<unsigned int>(date.day()));
}

template <class Clock, class Duration>
QDateTime toDateTime(const std::chrono::time_point<Clock, Duration>& tp)
{
    auto time = date::make_time(tp - std::chrono::floor<date::days>(tp));
    return QDateTime(toDate(tp), QTime(time.hours().count(), time.minutes().count(), time.seconds().count()));
}
}
