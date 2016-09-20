/*
 * DateTime.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include <string.h>
#include <time.h>
#include <iomanip>
#include <sstream>

#include "DateTime.h"
#include "TimeInterval.h"

namespace OPAQ {

DateTime::DateTime() {
    _year  = 0;
    _month = 0;
    _day   = 0;
    _hour  = 0;
    _min   = 0;
    _sec   = 0;
}

#ifdef WIN32
// strptime is not implemented on windows (http://stackoverflow.com/questions/321849/strptime-equivalent-on-windows)
static const char* strptime(const char* s, const char* f, struct tm* tm) {
    std::istringstream input(s);
    input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
    input >> std::get_time(tm, f);
    if (input.fail())
    {
        return nullptr;
    }
    return s + static_cast<size_t>(input.tellg());
}
#endif

DateTime::DateTime( const std::string& s ) {
	struct tm t;

	memset( &t, 0, sizeof(struct tm) );
	strptime( s.c_str(), "%Y-%m-%d %H:%M%S", &t );

	_year  = t.tm_year + 1900;
	_month = t.tm_mon + 1;
	_day   = t.tm_mday;
	_hour  = t.tm_hour;
	_min   = t.tm_min;
	_sec   = t.tm_sec;
}

const DateTime DateTime::operator+ (const TimeInterval &timeInterval) const {
	DateTime out = *this;
	out.addSeconds(timeInterval.getSeconds());
	return out;
}

const DateTime DateTime::operator- (const TimeInterval &timeInterval) const {
	DateTime out = *this;
	out.addSeconds(- timeInterval.getSeconds());
	return out;
}

bool DateTime::isValid() const {

	struct tm t;
	memset( &t, 0, sizeof(struct tm) );
	t.tm_year = _year - 1900;
	t.tm_mon  = _month-1;
	t.tm_mday = _day;
	t.tm_hour = _hour;
	t.tm_min  = _min;
	t.tm_sec  = _sec;

	return mktime(&t) >= 0;
}


}
