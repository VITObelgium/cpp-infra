/*
 * DataBuffer.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */ 

#ifndef OPAQ_DATA_BUFFER_H_
#define OPAQ_DATA_BUFFER_H_

#include <Component.h>
#include <vector>
#include "DataProvider.h"
#include "../TimeInterval.h"

namespace OPAQ {

class ForecastBuffer: public OPAQ::DataProvider {
public:
  ForecastBuffer();
  virtual ~ForecastBuffer();

  /**
   * Also return the basetime resolution
   */
  virtual TimeInterval getBaseTimeResolution( void ) = 0;


  /**
   * Set the nodata placeholder to use for incoming values,
   * alternatively you can also use the current nodata placeholder
   * as returned by getNoData()
   */
  virtual void setNoData( double noData ) = 0;

  /**
   * Store the given values with the given forecast horizons for given
   * ids.
   *
   * Only forecasted values are stored where the forecast horizons are relative
   * with respect to the basetime set in the DataProvider from which this 
   * class derives...
   *
   * This method is typically used by a forecast model to set the values
   */
  virtual void setValues( const DateTime &baseTime,
		  	  	  	  	  const OPAQ::TimeSeries<double>& forecast,
						  const std::string& stationId,
						  const std::string& pollutantId,
						  OPAQ::Aggregation::Type aggr ) = 0;

  /**
   * This routine retrieves the forecasted values for a specific base time
   * as a function of forecast horizon, given by the vector of time intervals
   */
  virtual OPAQ::TimeSeries<double> getValues( const DateTime &baseTime,
											  const std::vector<OPAQ::TimeInterval>& fc_hor,
											  const std::string& stationId,
											  const std::string& pollutantId,
											  OPAQ::Aggregation::Type aggr ) = 0;

  /**
   * This one gives the forecasts between the forecast times1 and 2 for a given fixed time lag (the
   * fc_hor. This routine can be used to e.g. retieve the archived day+2 forecasts for a given period
   * to e.g. calculate real time corrections
   */
  virtual OPAQ::TimeSeries<double> getValues( const OPAQ::TimeInterval fc_hor,
          	  	  	  	  	  	  	  	  	  const DateTime &fcTime1,
											  const DateTime &fcTime2,
											  const std::string& stationId,
											  const std::string& pollutantId,
											  OPAQ::Aggregation::Type aggr ) = 0;

  // need also routines for retrieving data for mapping, this
  // has to be
  // - for a given model and a given fcdate and fc horizon

private:
  
};
  
} /* namespace OPAQ */
#endif /* OPAQ_FORECAST_BUFFER_H_ */