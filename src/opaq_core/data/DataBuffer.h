/*
 * DataBuffer.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */ 

#ifndef OPAQ_DATA_BUFFER_H_
#define OPAQ_DATA_BUFFER_H_

#include <opaq/pfw.h>
#include <vector>
#include "DataProvider.h"
#include "../ForecastHorizon.h"

namespace OPAQ {

class DataBuffer: public OPAQ::DataProvider {
public:
  DataBuffer();
  virtual ~DataBuffer();

  /**
   * Set the nodata placeholder to use for incoming values,
   * alternatively you can also use the current nodata placeholder
   * as returned by getNoData()
   */
  virtual void setNoData(double noData) = 0;

  /**
   * Store the given values with the given forecast horizons for given
   * ids.
   *
   * Only forecasted values are stored where the forecast horizons are relative
   * with respect to the basetime set in the DataProvider from which this 
   * class derives...
   *
   * For output in an AQNetwork, these id are forecasted parameter & station.
   *
   * This method is typically used by a forecast model
   */
  virtual void setValues(const std::string & modelName, 
                         const std::vector<double> & values,
			 const std::vector<ForecastHorizon> & forecastHorizons,
			 const std::string & id1, 
			 const std::string & id2 ) = 0;


  /**
   * Store the given values, for the given id,  with the given forecast horizon
   * into the given grid (via setGridProvider)
   *
   * the id is a pollutant when mapping observations, or a forecasted parameter when 
   * mapping forecasts. Values contains a value for each cell in the grid
   *
   * This method is typically used by a mapping model
   */
  virtual void setValues(const std::string & modelName,
			 const std::vector<double> & values,
			 const std::string & id, 
			 const ForecastHorizon & forecastHorizon = ForecastHorizon(0)) = 0;
  
  /* TODO :
     Now we don't have a function which enables a user to set a number of basetimes
     i.e. with a beginOffset and an endOffset... 
   */
     

private:
  
};
  
} /* namespace OPAQ */
#endif /* OPAQ_DATA_BUFFER_H_ */
