/*
 * TimeInterval.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef TIMEINTERVAL_H_
#define TIMEINTERVAL_H_

#include "DateTime.h"

namespace OPAQ {

  /** 
      Class to represent a time interval
  */
  class TimeInterval {
  public:
    TimeInterval();
    virtual ~TimeInterval();
    
    enum Unit { Seconds, Minutes, Hours, Days };

    /** Construct a time interval from a number of seconds given */
    TimeInterval(long seconds) {
      this->_seconds = seconds;
    }

    /** Construct a time interval with some units given */
    TimeInterval( long value, TimeInterval::Unit unit );

    /** Construct a time interval from a begin date and an end date.
	Both begin and end date are given by a OPAQ::DateTime object
    */
    TimeInterval(const DateTime &begin, const DateTime &end) {
      this->_seconds = end.getUnixTime() - begin.getUnixTime();
    }

    /** Return the interval as seconds */
    long getSeconds () const {
      return _seconds;
    }
    
    /** Returns the interval as minutes */
    double getMinutes () const {
      return _seconds / 60.0;
    }

    /** Returns the interval as hours */
    double getHours () const {
      return _seconds / 3600.0;
    }

    /** Returns the interval as days */
    double getDays () const {
      return _seconds / 86400.0;
    }

    /**
     * operator+ overloaded
     */
    const TimeInterval operator+(const TimeInterval &other) const;

    /**
	 * operator- overloaded
	 */
	const TimeInterval operator-(const TimeInterval &other) const;

    /**
	 * operator!= overloaded
	 */
	bool operator!=(const TimeInterval &other) const;

    /**
	 * operator== overloaded
	 */
	bool operator==(const TimeInterval &other) const;


	/** A friendly output streamer */
	friend std::ostream& operator<<(std::ostream& os, const TimeInterval& d);

	/**
	 * operator* overloaded for multiplication with integer & unsigned integers
	 */
	friend TimeInterval operator*(int lhs, const TimeInterval& rhs );
	friend TimeInterval operator*(const TimeInterval& lhs, int rhs );
	friend TimeInterval operator*(unsigned int lhs, const TimeInterval& rhs );
	friend TimeInterval operator*(const TimeInterval& lhs, unsigned int rhs );

  private:
    long _seconds;
  };

} /* namespace OPAQ */
#endif /* TIMEINTERVAL_H_ */
