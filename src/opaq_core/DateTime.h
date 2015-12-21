#ifndef __OPAQ_DATETIME_H
#define __OPAQ_DATETIME_H

#include <ostream>
#include <sstream>
#include <iomanip>
#include <math.h>

namespace OPAQ {

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
  class DateTime {
  public:
    DateTime();

    /** Construct a datetime from a julian day number */
    DateTime(long jd) {
      setJulianDay (jd);
    }

    /** Construct a datetime from a given year, month and day */
    DateTime(int yyyy, int mm, int dd) :
      _year(yyyy), _month(mm), _day(dd), _hour(0), _min(0), _sec(0) {
    }

    /** Construct a datetime from a given year, month, day as well as hours, minutes and seconds */
    DateTime(int yyyy, int mm, int dd, int hour, int min, int sec) :
      _year(yyyy), _month(mm), _day(dd), _hour(hour), _min(min), _sec(sec) {
    }

    /** Construct a datetime from a given string, format should be YYYY-MM-DD HH:MM:SS, the
     * HH:MM:SS can be omitted
     */
    DateTime( const std::string& s );

    ~DateTime() {
    }
    
    /**
     * Checks whether the date/time is actually valid
     */
    bool isValid() const;

    /**
       Given a julian day number, the year, month and day are set. 

       JD = NNNNNNNN is the julian date from an epoch
       in the very distant past. 

       See CACM 1968 11(10):657, LETTER TO THE EDITOR BY FLIEGEL AND VAN FLANDERN.
     */    
    void setJulianDay (long jd) {
      int l, n;
		
      l = jd + 68569;
      n = 4 * l / 146097;
      l = l - (146097 * n + 3) / 4;
      _year = 4000 * (l + 1) / 1461001;
      l = l - 1461 * _year / 4 + 31;
      _month = 80 * l / 2447;
      _day = l - 2447 * _month / 80;
      l = _month / 11;
      _month = _month + 2 - 12 * l;
      _year = 100 * (n - 49) + _year + l;
      
      _hour = 12;		// julian day refers to noon
      _min = 0;
      _sec = 0;
    }

    /** Set the datetime seconds */
    void setSec(int s) { _sec = s;  }
    /** Set the datetime minuts */
    void setMin(int m) { _min = m;  }
    /** Set the datetime hours */
    void setHour(int h) { _hour = h; }
    /** Set the datetime day */
    void setDay(int dd) { _day = dd;  }
    /** Set the datetime month */
    void setMonth(int mm) { _month = mm;  }
    /** Set the datetime year */
    void setYear(int yyyy) { _year = yyyy; }
    
    /** Return the datetime seconds */
    int getSec() const { return _sec; }
    /** Return the datetime minuts */
    int getMin() const { return _min; }
    /** Return the datetime hour */
    int getHour() const { return _hour; }
    /** Return the datetime day */
    int getDay() const { return _day; }
    /** Return the datetime month */
    int getMonth() const { return _month; }
    /** Return the datetime year */
    int getYear() const { return _year; }
    
    /**
       Converts the datetime object to a julian date number which it returns
       See Fliegel & Van Flandern, CACM 11(10):657, 1968
       example: julian_date(1970,1,1)=2440588
     */

    long toJulianDate() const {
      return _day - 32075 + 1461 * (_year + 4800 + (_month - 14) / 12) / 4
	+ 367 * (_month - 2 - ((_month - 14) / 12) * 12) / 12
	- 3 * ((_year + 4900 + (_month - 14) / 12) / 100) / 4;
    }
    
    /**
     * Returns the UNIX time from the datetime object
     * \author Stijn Van Looy, (c) 2014 VITO
     * A bit different from the formula @ http://en.wikipedia.org/wiki/Julian_day
     * since in this implementation the julian 'date' is actually the julian _day_
     * (that is: an integer counting the days from noon to noon, and not a floating
     * point number of which the integer part is the julian day and the decimal
     * part denotes the time passed since noon that day.
     */
    long getUnixTime() const {
      return (toJulianDate() - 2440588) * 86400.0
	+ _hour * 3600 + _min * 60 + _sec;
    }
    
    /**
       Set the datetime to the given unix time
       \note The UNIX time is the number of seconds since 01-01-1970 00:00:00
       \author Stijn Van Looy, (c) VITO 2014
    */
       
    void setUnixTime(long unixTime) {
      long midnight = floor ((double) unixTime / 86400.0) * 86400;
      long jd = midnight / 86400 + 2440588;
      setJulianDay(jd);
      long delta = unixTime - midnight;
      _hour = delta / 3600;
      delta -= _hour * 3600;
      _min = delta / 60;
      _sec = delta - _min * 60;
    }

    /**
     * Add/subtract a number of seconds to/from the datetime
     */
    void addSeconds (int count) {
      // (c) 2014 Stijn.VanLooy@vito.be
      long unixTime = getUnixTime();
      unixTime += count;
      setUnixTime(unixTime);
    }
    
    /**
     * Add/subtract a number of minutes to/from the datetime
     */
    void addMinutes (int count) {
      // (c) 2014 Stijn.VanLooy@vito.be
      addSeconds (count * 60);
    }
    
    /**
     * Add/subtract a number of hours to/from the datetime
     */
    void addHours (int count) {
      // (c) 2014 Stijn.VanLooy@vito.be
      addSeconds (count * 3600);
    }
    
    /**
     * Add/subtract a number of days to/from the datetime
     */
    void addDays (int count) {
      // (c) 2014 Stijn.VanLooy@vito.be
      addSeconds (count * 86400);
    }
    
    /** 
	Return the day of the week for the datetime object
	Day_Of_Week: (0=Sunday,1=Monday...6=Saturday)
	cf J.D.Robertson, CACM 15(10):918
    */
    int getDayOfWeek() const {
      return ((13 * (_month + 10 - (_month + 10) / 13 * 12) - 1) / 5 + _day
	      + 77 + 5 * (_year + (_month - 14) / 12
			  - (_year + (_month - 14) / 12) / 100 * 100) / 4
	      + (_year + (_month - 14) / 12) / 400
	      - (_year + (_month - 14) / 12) / 100 * 2 ) % 7;
    }  


    /**
       Convert the datetime to a string for easy output
       Format is YYYY-MM-DD HH:MM:SS
    */
    std::string toString() const {
      std::ostringstream s;
      s << (*this);
      return s.str();
    }
    
    /**
       Convert only the date and not full datetime to string for easy output
       format is YYYY-MM-DD
    */
    std::string dateToString() const {
      std::ostringstream s;
      DateTime d( this->getYear(), this->getMonth(), this->getDay() );
      s << std::setw(4) << d._year << "-" << std::setw(2) << std::setfill('0')
       << d._month << std::setw(1) << "-" << std::setw(2)
       << std::setfill('0') << d._day;
      return s.str();
    }

    /** A friendly output streamer */
    friend std::ostream& operator<<(std::ostream& os, const DateTime& d);

    /**
     * Comparison operator overloading
     * \author Stijn Van Looy (c) VITO 2014
     */
    inline bool operator==(const DateTime& rhs) const {
      return this->getUnixTime() == rhs.getUnixTime();
    }
    inline bool operator!=(const DateTime& rhs) const{
      return !(*this == rhs);
    }
    inline bool operator< (const DateTime& rhs) const {
      return this->getUnixTime() < rhs.getUnixTime();
    }
    inline bool operator> (const DateTime& rhs) const {
      return  rhs < *this;
    }
    inline bool operator<=(const DateTime& rhs) const {
      return !(*this > rhs);
    }
    inline bool operator>=(const DateTime& rhs) const {
      return !(*this < rhs);
    }
    
    /**
     * operator+ overloaded: allows adding a time interval to the date time
     */
    const DateTime operator+ (const TimeInterval &timeInterval) const;

    /**
	 * operator- overloaded: allows substracting a time interval from the date time
	 */
	const DateTime operator- (const TimeInterval &timeInterval) const;

  private:
    int _year; /* yyyy */
    int _month; /* 1 -> 12 */
    int _day;
    int _hour;
    int _min;
    int _sec;
  };
  
  inline std::ostream& operator<<(std::ostream& os, const DateTime& d) {
    os << std::setw(4) << d._year << "-" << std::setw(2) << std::setfill('0')
       << d._month << std::setw(1) << "-" << std::setw(2)
       << std::setfill('0') << d._day << std::setw(1) << " "
       << std::setw(2) << std::setfill('0') << d._hour << std::setw(1)
       << ":" << std::setw(2) << std::setfill('0') << d._min
       << std::setw(1) << ":" << std::setw(2) << std::setfill('0')
       << d._sec;
    return os;
  }
  
}

#endif /* #ifndef __OPAQ_DATETIME_H */
