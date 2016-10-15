#ifndef __OPAQ_DATETIME_H
#define __OPAQ_DATETIME_H

#include <iomanip>
#include <math.h>
#include <ostream>
#include <sstream>

#include <ctime>
#include <cstring>
#include <cassert>

namespace OPAQ
{

// forward declaration to avoid circular header references
class TimeInterval;

/**
     Class representing a DateTime object
     The class contains some general functionality to deal with date/times in OPAQ. It contains
     some routines for conversion to a julian date (having sequential date numbers ) and unix time
     being the number of seconds since 1970-01-01 00:00:00 UTC.
     \note That OPAQ is completely unaware of the timezone and contains no functionality for
           timezone conversion. This has to be handled by the dataproviders in the plugins or the
	   preprocessing of the data fed into OPAQ.
   */
class DateTime
{
public:
    DateTime();

    DateTime(time_t t);

    /** Construct a datetime from a given year, month and day */
    DateTime(int yyyy, int mm, int dd)
    : DateTime(yyyy, mm, dd, 0, 0, 0)
    {
    }

    /** Construct a datetime from a given year, month, day as well as hours, minutes and seconds */
    DateTime(int yyyy, int mm, int dd, int hour, int min, int sec)
    {
        tm t;
        memset(&t, 0, sizeof(struct tm));
        t.tm_year = yyyy - 1900;
        t.tm_mon = mm - 1;
        t.tm_mday = dd;
        t.tm_hour = hour;
        t.tm_min = min;
        t.tm_sec = sec;

        _time = mktime(&t);
    }

    /** Construct a datetime from a given string, format should be YYYY-MM-DD HH:MM:SS, the
     * HH:MM:SS can be omitted
     */
    DateTime(const std::string& s);

    /**
     * Checks whether the date/time is actually valid
     */
    bool isValid() const;

    int getSec() const;
    int getMin() const;
    int getHour() const;
    int getDay() const;
    int getMonth() const;
    int getYear() const;

    /**
       Converts the datetime object to a julian date number which it returns
       See Fliegel & Van Flandern, CACM 11(10):657, 1968
       example: julian_date(1970,1,1)=2440588
     */

    /**
     * Returns the UNIX time from the datetime object
     * \author Stijn Van Looy, (c) 2014 VITO
     * A bit different from the formula @ http://en.wikipedia.org/wiki/Julian_day
     * since in this implementation the julian 'date' is actually the julian _day_
     * (that is: an integer counting the days from noon to noon, and not a floating
     * point number of which the integer part is the julian day and the decimal
     * part denotes the time passed since noon that day.
     */
    time_t getUnixTime() const
    {
        return _time;
    }

    /**
       Set the datetime to the given unix time
       \note The UNIX time is the number of seconds since 01-01-1970 00:00:00
       \author Stijn Van Looy, (c) VITO 2014
    */

    void setUnixTime(time_t time)
    {
        _time = time;
    }

    /**
     * Add/subtract a number of seconds to/from the datetime
     */
    void addSeconds(int count)
    {
        _time += count;
    }

    /**
     * Add/subtract a number of minutes to/from the datetime
     */
    void addMinutes(int count)
    {
        addSeconds(count * 60);
    }

    /**
     * Add/subtract a number of hours to/from the datetime
     */
    void addHours(int count)
    {
        addSeconds(count * 3600);
    }

    /**
     * Add/subtract a number of days to/from the datetime
     */
    void addDays(int count)
    {
        addSeconds(count * 86400);
    }

    /**
	Return the day of the week for the datetime object
	Day_Of_Week: (0=Sunday,1=Monday...6=Saturday)
	cf J.D.Robertson, CACM 15(10):918
    */
    int getDayOfWeek() const
    {
        auto* t = std::gmtime(&_time);
        assert(t);
        return t->tm_wday;
    }

    /**
       Convert the datetime to a string for easy output
       Format is YYYY-MM-DD HH:MM:SS
    */
    std::string toString() const
    {
        std::ostringstream s;
        s << (*this);
        return s.str();
    }

    /**
       Convert only the date and not full datetime to string for easy output
       format is YYYY-MM-DD
    */
    std::string dateToString() const
    {
        auto* t = std::gmtime(&_time);
        assert(t);

        std::ostringstream s;
        s << std::setw(4) << t->tm_year + 1900 << "-" << std::setw(2) << std::setfill('0')
          << t->tm_mon + 1 << std::setw(1) << "-" << std::setw(2)
          << std::setfill('0') << t->tm_mday;

        return s.str();
    }

    /** A friendly output streamer */
    friend std::ostream& operator<<(std::ostream& os, const DateTime& d);

    /**
     * Comparison operator overloading
     * \author Stijn Van Looy (c) VITO 2014
     */
    inline bool operator==(const DateTime& rhs) const
    {
        return _time == rhs._time;
    }
    inline bool operator!=(const DateTime& rhs) const
    {
        return !(*this == rhs);
    }
    inline bool operator<(const DateTime& rhs) const
    {
        return _time < rhs._time;
    }
    inline bool operator>(const DateTime& rhs) const
    {
        return rhs < *this;
    }
    inline bool operator<=(const DateTime& rhs) const
    {
        return !(*this > rhs);
    }
    inline bool operator>=(const DateTime& rhs) const
    {
        return !(*this < rhs);
    }

    /**
     * operator+ overloaded: allows adding a time interval to the date time
     */
    const DateTime operator+(const TimeInterval& timeInterval) const;

    /**
	 * operator- overloaded: allows substracting a time interval from the date time
	 */
    const DateTime operator-(const TimeInterval& timeInterval) const;

private:
    time_t _time;
};

inline std::ostream& operator<<(std::ostream& os, const DateTime& d)
{
    auto* t = std::gmtime(&d._time);
    assert(t);

    os << std::setw(4) << t->tm_year + 1900 << "-" << std::setw(2) << std::setfill('0')
       << t->tm_mon + 1 << std::setw(1) << "-" << std::setw(2)
       << std::setfill('0') << t->tm_mday << std::setw(1) << " "
       << std::setw(2) << std::setfill('0') << t->tm_hour << std::setw(1)
       << ":" << std::setw(2) << std::setfill('0') << t->tm_min
       << std::setw(1) << ":" << std::setw(2) << std::setfill('0')
       << t->tm_sec;
    return os;
}
}

#endif /* #ifndef __OPAQ_DATETIME_H */
