/*
 * DateTime.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "DateTime.h"
#include "TimeInterval.h"

namespace OPAQ {

DateTime::DateTime() {}

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
