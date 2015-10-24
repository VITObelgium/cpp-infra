/*
 * TimeInterval.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "TimeInterval.h"

namespace OPAQ {

TimeInterval::TimeInterval() {}

TimeInterval::~TimeInterval() {}

const TimeInterval TimeInterval::operator+(const TimeInterval &other) const {
	TimeInterval out = *this;
	out._seconds += other.getSeconds();
	return out;
}

const TimeInterval TimeInterval::operator-(const TimeInterval &other) const {
	TimeInterval out = *this;
	out._seconds -= other.getSeconds();
	return out;
}

} /* namespace OPAQ */
