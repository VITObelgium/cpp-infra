/*
 * DateTime.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include <time.h>

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
	t.tm_year = _year - 1900;
	t.tm_mon  = _month-1;
	t.tm_mday = _day;
	t.tm_hour = _hour;
	t.tm_min  = _min;
	t.tm_sec  = _sec;

	if ( mktime( &t ) < 0 ) return false;
	return true;
}


}
