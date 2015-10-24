/*
 * ForecastHorizon.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef FORECASTHORIZON_H_
#define FORECASTHORIZON_H_

#include "TimeInterval.h"

namespace OPAQ {

  /**
     Class to represent a forecast horizon, derived from a Timeinterval
     This class derives from a generic timeinterval, adding the possibility
     to construct it via giving a number of hours. 
   */
  class ForecastHorizon: public OPAQ::TimeInterval {
  public:
    ForecastHorizon();
    virtual ~ForecastHorizon();
    
    /** Construct a forecast horizon for a given number of hours */
    ForecastHorizon(long hours) : TimeInterval (hours * 3600) {}

    /** Returns the forecast horizon in hours */
    long getHours() const {
      return getSeconds() / 3600;
    }
  };

} /* namespace OPAQ */
#endif /* FORECASTHORIZON_H_ */
