/*
 * DataProvider.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef OPAQ_DATA_PROVIDER_H_
#define OPAQ_DATA_PROVIDER_H_

#include <vector>
#include "../Component.h"
#include "../TimeSeries.h"
#include "../AQNetworkProvider.h"
#include "../DateTime.h"
#include "../TimeInterval.h"
#include "../Aggregation.h"

namespace OPAQ {

/**
 * Observation data provider
 */
class DataProvider: public OPAQ::Component {
public:
  DataProvider();
  virtual ~DataProvider();


  // ==================================================================================
  // PUBLIC METHODS
  // ==================================================================================
  /**
   * Set the Air Quality Network provider
   */
  void setAQNetworkProvider( AQNetworkProvider *net ) { _AQNetworkProvider = net; };

  /**
   * Sets the name of the current active model in the buffer. Not used for observation
   * data, only for model data...
   */
  void setCurrentModel( const std::string& modelName ) { _currentModel = modelName; }

  /**
   * Returns the name of the current active model, not used for observation data
   */
  const std::string& getCurrentModel( void ){ return _currentModel; }


  // ==================================================================================
  // PURE VIRTUAL METHODS
  // ==================================================================================
  /**
   * Get the time resolution of the provided data, for forecasted data this will be the time resolution
   * of the forecasts
   */
  virtual TimeInterval getTimeResolution() = 0;

  /**
   * Get the nodata placeholder
   */
  virtual double getNoData() = 0;

  /**
   * Return an aggregated version of the base data in there..
   */
  virtual OPAQ::TimeSeries<double> getValues( const DateTime& t1,
  		  	  	  	  	  	  	  	  	  	  const DateTime& t2,
  											  const std::string& stationId,
  											  const std::string& pollutantId,
											  OPAQ::Aggregation::Type aggr = OPAQ::Aggregation::None ) = 0;

  /**
   * This method would typically be used by a mapping model
   */
  // need routine to return a vector (or a map w.r.t. station) for a given date over all the stations
  // virtual std::vector<double>
  // virtual std::map<std::string, double>
  // or even a timeseries :
  // virtual OPAQ::TimeSeries<std::map<std::string, double> > to conduct the interpolation...
  // or a map between station pointers and the values ?, or even a time series thereof...
  // virtual std::map<OPAQ::Station*, double> getValues( const DateTime& t );

protected:
  std::string        _currentModel;       //! name of the current model which provides the data, not used for observations
  AQNetworkProvider *_AQNetworkProvider;  //! the network provider
};

} /* namespace OPAQ */
#endif /* OPAQ_DATA_PROVIDER_H_ */
