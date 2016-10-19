/*
 * DateTime.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include <iomanip>
#include <sstream>
#include <cstring>
#include <ctime>
#include <cassert>

#include "DateTime.h"
#include "Exceptions.h"
#include "TimeInterval.h"

namespace OPAQ
{

DateTime::DateTime()
: _time(-1)
{
}

DateTime::DateTime(time_t t)
: _time(t)
{
}

#ifdef WIN32
// strptime is not implemented on windows (http://stackoverflow.com/questions/321849/strptime-equivalent-on-windows)
static const char* strptime(const char* s, const char* f, struct tm* tm)
{
    std::stringstream input(s);
    input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
    input >> std::get_time(tm, f);
    if (input.fail())
    {
        return nullptr;
    }
    return s + static_cast<size_t>(input.tellg());
}
#endif

DateTime::DateTime(const std::string& s)
{
    std::tm t;

    memset(&t, 0, sizeof(struct tm));
    //strptime( s.c_str(), "%Y-%m-%d %H:%M%S", &t );
    strptime(s.c_str(), "%Y-%m-%d", &t);

    _time = mktime(&t);
}

const DateTime DateTime::operator+(const TimeInterval& timeInterval) const
{
    DateTime out = *this;
    out.addSeconds(timeInterval.getSeconds());
    return out;
}

const DateTime DateTime::operator-(const TimeInterval& timeInterval) const
{
    DateTime out = *this;
    out.addSeconds(-timeInterval.getSeconds());
    return out;
}

DateTime& DateTime::operator+=(const TimeInterval& ti)
{
    addSeconds(ti.getSeconds());
    return *this;
}

DateTime& DateTime::operator-=(const TimeInterval& ti)
{
    addSeconds(-ti.getSeconds());
    return *this;
}

bool DateTime::isValid() const
{
    return _time != -1;
}

int DateTime::getSec() const
{
    auto* t = std::gmtime(&_time);
    assert(t);
    return t->tm_sec;
}

int DateTime::getMin() const
{
    auto* t = std::gmtime(&_time);
    assert(t);
    return t->tm_min;
}

int DateTime::getHour() const
{
    auto* t = std::gmtime(&_time);
    assert(t);
    return t->tm_hour;
}

int DateTime::getDay() const
{
    auto* t = std::gmtime(&_time);
    assert(t);
    return t->tm_mday;
}

int DateTime::getMonth() const
{
    auto* t = std::gmtime(&_time);
    assert(t);
    return t->tm_mon + 1;
}

int DateTime::getYear() const
{
    auto* t = gmtime(&_time);
    if (t == nullptr)
    {
        throw RunTimeException("Invalid time structure");
    }

    return t->tm_year + 1900;
}

}
