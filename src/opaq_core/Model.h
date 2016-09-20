/*
 * Interfaces.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#ifndef OPAQ_MODEL_H_
#define OPAQ_MODEL_H_

#include <tinyxml.h>
#include <string>

#include "Component.h"

#include "data/DataProvider.h"
#include "data/MeteoProvider.h"
#include "data/ForecastBuffer.h"
#include "data/GridProvider.h"

#include "AQNetworkProvider.h"
#include "Pollutant.h"

namespace OPAQ {

// forward declaration
class Component;

class Model: virtual public OPAQ::Component {
public:
  Model();
  virtual ~Model() {
  }

  virtual void setBaseTime(DateTime & baseTime) {
    this->baseTime = baseTime;
  }
  virtual void setPollutant(Pollutant & pollutant) {
    this->pollutant = pollutant;
  }
  virtual void setAggregation( Aggregation::Type aggr ) {
	  this->aggregation = aggr;
  }
  virtual void setForecastHorizon( const TimeInterval& forecastHorizon) {
    this->forecastHorizon = forecastHorizon;
  }
  virtual void setAQNetworkProvider( AQNetworkProvider *aqNetworkProvider) {
    this->aqNetworkProvider = aqNetworkProvider;
  }
  virtual void setGridProvider(GridProvider * gridProvider) {
    this->gridProvider = gridProvider;
  }
  virtual void setInputProvider(DataProvider * input) {
    this->input = input;
  }
  virtual void setMeteoProvider(MeteoProvider * meteo) {
    this->meteo = meteo;
  }
  virtual void setBuffer(ForecastBuffer * buffer) {
    this->buffer = buffer;
  }
  
  virtual const DateTime & getBaseTime() {
    return baseTime;
  }
  virtual const Pollutant & getPollutant() {
    return pollutant;
  }
  virtual const Aggregation::Type & getAggregation() {
	  return aggregation;
  }
  virtual const TimeInterval& getForecastHorizon() {
    return forecastHorizon;
  }

  // Throws NullPointerException
  virtual AQNetworkProvider * getAQNetworkProvider() {
    if (aqNetworkProvider == NULL) throw NullPointerException();
    return aqNetworkProvider;
  }

  // Throws NullPointerException
  virtual GridProvider * getGridProvider() {
    if (gridProvider == NULL) throw NullPointerException();
    return gridProvider;
  }

  // Throws NullPointerException
  virtual DataProvider * getInputProvider() {
    if (input == NULL) throw NullPointerException();
    return input;
  }

  // Throws NullPointerException
  virtual MeteoProvider * getMeteoProvider() {
    if (meteo == NULL) throw NullPointerException();
    return meteo;
  }

  // Throws NullPointerException
  virtual ForecastBuffer * getBuffer() {
    if (buffer == NULL) throw NullPointerException();
    return buffer;
  }
  
  virtual void run() = 0;
  
  int getNoData() { return missing_value; }
  void setNoData( int missing ) { missing_value = missing; }

protected:
  DateTime           baseTime;        //< run for this basetime
  Pollutant          pollutant;       //< run for this pollutant
  Aggregation::Type  aggregation;     //< run for this aggregation
  TimeInterval       forecastHorizon; //< maximum forecast horizon to run to

  AQNetworkProvider *aqNetworkProvider;
  GridProvider      *gridProvider;
  DataProvider      *input;
  MeteoProvider     *meteo;
  ForecastBuffer    *buffer;

  int missing_value;                  //!< missing value, can be set in configuration, default set here
};

} /* namespace opaq */
#endif /* OPAQ_MODEL_H_ */
