#pragma once

#include "infra/chrono.h"

#include <qdatetime.h>

namespace inf::ui {

template <class Clock, class Duration>
QDate toDate(const std::chrono::time_point<Clock, Duration>& tp)
{
#if __cplusplus > 201703L
    namespace date = std::chrono;
#endif

    date::year_month_day d = inf::chrono::to_year_month_day(tp);
    return QDate(int(d.year()), static_cast<unsigned int>(d.month()), static_cast<unsigned int>(d.day()));
}

template <class Clock, class Duration>
QDateTime toDateTime(const std::chrono::time_point<Clock, Duration>& tp)
{
    auto time = inf::chrono::time_of_day(tp);
    return QDateTime(toDate(tp), QTime(time.hours().count(), time.minutes().count(), time.seconds().count()));
}

inline inf::chrono::date_point datePointFromDate(const QDate& d)
{
#if __cplusplus > 201703L
    namespace date = std::chrono;
#endif

    const date::year_month_day ymd(date::year(d.year()),
                                   date::month(d.month()),
                                   date::day(d.day()));
    return static_cast<date::sys_days>(ymd);
}

inline inf::chrono::local_date_point localDatePointFromDate(const QDate& d)
{
#if __cplusplus > 201703L
    namespace date = std::chrono;
#endif

    const date::year_month_day ymd(date::year(d.year()),
                                   date::month(d.month()),
                                   date::day(d.day()));
    return static_cast<date::local_days>(ymd);
}
}
