/*
 * TimeInterval.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "Exceptions.h"
#include "TimeInterval.h"

namespace OPAQ {

TimeInterval::TimeInterval() {
	_seconds = 0;
}

TimeInterval::~TimeInterval() {}

TimeInterval::TimeInterval( long value, TimeInterval::Unit unit ) {

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

bool TimeInterval::operator!=(const TimeInterval &other) const {
	return ( this->_seconds != other._seconds );
}

bool TimeInterval::operator==(const TimeInterval &other) const {
	return ( this->_seconds == other._seconds );
}

std::ostream& operator<<(std::ostream& os, const TimeInterval& d) {
    long days = d.getDays();
    long hours = d.getHours() - (days * 24);
    long minutes = d.getMinutes() - (days * 24 * 60) - (hours * 60);
    long seconds = d.getSeconds() - (days * 24 * 60 * 60) - (hours * 60 * 60) - (minutes * 60);
    os << days << " days, " << std::setw(2) << std::setfill('0') << hours
       << std::setw(1) << ":" << std::setw(2) << minutes
       << std::setw(1) << ":" << std::setw(2) << seconds;
    return os;
}

TimeInterval operator*(int lhs, const TimeInterval& rhs ) {
	TimeInterval out = rhs;
	out._seconds *= lhs;
	return out;
}

TimeInterval operator*(const TimeInterval& lhs, int rhs ) {
	return rhs * lhs;
}

TimeInterval operator*(unsigned int lhs, const TimeInterval& rhs ) {
	TimeInterval out = rhs;
	out._seconds *= lhs;
	return out;
}

TimeInterval operator*(const TimeInterval& lhs, unsigned int rhs ) {
	return rhs * lhs;
}

} /* namespace OPAQ */
