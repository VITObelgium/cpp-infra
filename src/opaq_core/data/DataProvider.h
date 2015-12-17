/*
 * DataProvider.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef OPAQ_DATA_PROVIDER_H_
#define OPAQ_DATA_PROVIDER_H_

#include <vector>
#include <Component.h>
#include "../DateTime.h"
#include "../TimeInterval.h"
#include "../ForecastHorizon.h"

namespace OPAQ {

class DateTime;
class AQNetworkProvider;

class DataProvider: public OPAQ::Component {
public:
  DataProvider();
  virtual ~DataProvider();

  /**
   * Set the Air Quality Network provider
   * Only needed for observation data, ignored for other data
   */
  virtual void setAQNetworkProvider( AQNetworkProvider * aqNetworkProvider) = 0;

  /**
   * Set the base time
   */
  virtual void setBaseTime(const DateTime & baseTime)
    throw (BadConfigurationException) = 0;
  
  /**
   * Get the time resolution of the provided data
   */
  virtual TimeInterval getTimeResolution() = 0;
  
  /**
   * Get the range within wich the provider operates
   * All requested data beyond this range will contain
   * the nodata placeholder
   *
   * The range is given relative to the base time
   * for example: if (-5 days, 4 days) is returned,
   * 	then date is available from 5 days before the base time until
   * 	4 days after the base time, both boundaries included.
   *
   * the returned range applies to forecast horizon = 0
   */
  virtual std::pair<const TimeInterval, const TimeInterval> getRange() = 0;
  
  /**
   * Same as getRange() but for a given forecast horizon
   */
  virtual std::pair<const TimeInterval, const TimeInterval> 
    getRange(const ForecastHorizon & forecastHorizon) = 0;

  /**
   * Get the size of the data vectors returned by the getValues methods
   * when begin and end are set to the ones returned by getRange()
   */
  virtual unsigned int size() = 0;

  /**
   * Get the nodata placeholder
   */
  virtual double getNoData() = 0;

  /**
   * Get the nodata placeholder for the given id
   * For example: for meteo data, the no data placeholder might be different for each
   * parameter.
   */
  virtual double getNoData(const std::string & id) = 0;

  /**
   * Get the values within a given interval, for given ids,
   * and the default forecast horizon (0)
   *
   * The interval begin and end offsets are given relative to the base time.
   * For example: if beginOffset = -5 days, endOffset = 0 days and getTimeResolution returns 1 day,
   * 	then values are returned for bt - 5 days, bt - 4 days, .. , bt - 1 day, and bt
   *
   * Each Data Provider has a given time resolution. If beginOffset and endOffset are
   * chosen exactly on the interval boundaries, it is clear which values are returned.
   * Things are a bit different if the offsets are not exactly on those boundaries, but
   * (say) a few seconds off.
   * Here we assume that if the user of the data provider sets an offset to (say) 8 hours,
   * he does want the time stamp "base time + 8 hours" to be included in the interval for
   * which values are requested. Therefore the beginOffset will be rounded down to the
   * nearest exact interval boundary, and the endOffset will be rounded up to the nearest
   * exact interval boundary.
   *
   * However "included in the interval" can mean different things. If the values represent
   * momentary measurements, there simply is no value for the given time stamp. In this case
   * it is unclear which value must be included and which not (rounded up or down)
   * If the values represent (say) averaged concentrations, the value holds for
   * the entire interval. Which value must be included in the interval depends on which
   * of the values (start or end of the interval) applies to the interval.
   * Therefore it stays important that the user choses the beginOffset and endOffset
   * values carefully.
   * The getTimeResolution method can come in handy here. If for example and entire day
   * of data is required, one can set beginOffset to zero, and endOffset to
   * beginOffset + TimeInterval(24 * 3600) - getTimeResolution()
   *
   * For observations data, these ids are pollutant & station,
   * while for meteo data, these ids are meteo id & meteo parameter,
   * and for (historical) forecast data, these ids are forecasted parameter & station
   *
   * This method is typically used by a forecast model
   */
  virtual std::vector<double> getValues(const TimeInterval & beginOffset,
					const TimeInterval & endOffset, 
					const std::string & id1,
					const std::string & id2 ) = 0;

  /**
   * Same as the normal getValues routine, but for the model of which the name is given...
   */
  virtual std::vector<double> getValues(const std::string & modelName, 
					const TimeInterval & beginOffset,
					const TimeInterval & endOffset, 
					const std::string & id1,
					const std::string & id2 ) = 0;

  
  /**
   * Get the values within a given interval, for given ids,
   * and for the given forecast horizon
   *
   * For observations data, these ids are pollutant & station,
   * while for meteo data, these ids are meteo id & meteo parameter,
   * and for (historical) forecast data, these ids are forecasted parameter & station
   *
   * Also read the remark about beginOffset and endOffset in
   * getValues(TimeInterval, TimeInterval, std::string, std::string)
   *
   * This method is typically used by a forecast model
   */
  virtual std::vector<double> getValues(const TimeInterval & beginOffset,
					const TimeInterval & endOffset, 
					const std::string & id1,
					const std::string & id2,
					const ForecastHorizon & forecastHorizon) = 0;


  virtual std::vector<double> getValues(const std::string & modelName, 
					const TimeInterval & beginOffset,
					const TimeInterval & endOffset, 
					const std::string & id1,
					const std::string & id2,
					const ForecastHorizon & forecastHorizon) = 0;

  
  /**
   * Get the values for given time (= base time + offset),
   * for the given id and forecast horizon,
   * and for all stations in the AQ network
   *
   * The id is a pollutant for observations, or a forecasted parameter for forecasts
   *
   * This method is typically used by a mapping model
   */
  virtual std::vector<double> getValues(const std::string & id,
					const TimeInterval & offset = TimeInterval(0),
					const ForecastHorizon & forecastHorizon = 
					 ForecastHorizon(0) ) = 0;

};

} /* namespace OPAQ */
#endif /* OPAQ_DATA_PROVIDER_H_ */
