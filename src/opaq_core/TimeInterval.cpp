/*
 * TimeInterval.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "TimeInterval.h"

namespace OPAQ {

TimeInterval::TimeInterval() {
	_seconds = 0;
}

TimeInterval::~TimeInterval() {}

void TimeInterval::TimeInterval( long value, TimeInterval::Unit unit ) {

	switch( unit ) {
	case TimeInterval::Seconds:
		this->_seconds = value;
		break;
    case TimeInterval::Minutes:
    	this->_seconds = 60*value;
    	break;
    case TimeInterval::Hours:
    	this->_seconds = 3600*value;
    	break;
    case TimeInterval::Days:
    	this->_seconds = 86400*value;
    	break;
    default:
    	throw RunTimeException( "wrong time interval given" );
    }
	return;
}


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
