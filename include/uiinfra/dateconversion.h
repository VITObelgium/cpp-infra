#pragma once

#include "infra/chrono.h"

#include <date/date.h>
#include <qdatetime.h>

namespace inf::ui {

template <class Clock, class Duration>
QDate toDate(const std::chrono::time_point<Clock, Duration>& tp)
{
    date::year_month_day date = inf::chrono::to_year_month_day(tp);
    return QDate(int(date.year()), static_cast<unsigned int>(date.month()), static_cast<unsigned int>(date.day()));
}

template <class Clock, class Duration>
QDateTime toDateTime(const std::chrono::time_point<Clock, Duration>& tp)
{
    auto time = inf::chrono::time_of_day(tp);
    return QDateTime(toDate(tp), QTime(time.hours().count(), time.minutes().count(), time.seconds().count()));
}

inline inf::chrono::date_point datePointFromDate(const QDate& date)
{
    const date::year_month_day ymd(date::year(date.year()),
                                   date::month(date.month()),
                                   date::day(date.day()));
    return static_cast<date::sys_days>(ymd);
}

inline inf::chrono::local_date_point localDatePointFromDate(const QDate& date)
{
    const date::year_month_day ymd(date::year(date.year()),
                                   date::month(date.month()),
                                   date::day(date.day()));
    return static_cast<date::local_days>(ymd);
}
}
