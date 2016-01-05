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
#include <Component.h>

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
  virtual AQNetworkProvider * getAQNetworkProvider() throw (NullPointerException) {
    if (aqNetworkProvider == NULL) throw NullPointerException();
    return aqNetworkProvider;
  }
  virtual GridProvider * getGridProvider() throw (NullPointerException) {
    if (gridProvider == NULL) throw NullPointerException();
    return gridProvider;
  }
  virtual DataProvider * getInputProvider() throw (NullPointerException) {
    if (input == NULL) throw NullPointerException();
    return input;
  }
  virtual MeteoProvider * getMeteoProvider() throw (NullPointerException) {
    if (meteo == NULL) throw NullPointerException();
    return meteo;
  }
  virtual ForecastBuffer * getBuffer() throw (NullPointerException) {
    if (buffer == NULL) throw NullPointerException();
    return buffer;
  }
  
  virtual void run() = 0;
  
private:
  DateTime           baseTime;        //< run for this basetime
  Pollutant          pollutant;       //< run for this pollutant
  Aggregation::Type  aggregation;     //< run for this aggregation
  TimeInterval       forecastHorizon; //< maximum forecast horizon to run to

  AQNetworkProvider *aqNetworkProvider;
  GridProvider      *gridProvider;
  DataProvider      *input;
  MeteoProvider     *meteo;
  ForecastBuffer    *buffer;
};

} /* namespace opaq */
#endif /* OPAQ_MODEL_H_ */
