/*
 * DateTime.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

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

}
