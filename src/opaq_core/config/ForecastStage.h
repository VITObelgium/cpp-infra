/*
 * Plugin.h
 *
 *  Created on: Jan 9, 2014
 *      Author: bino.maiheu@vito.be
 */

#ifndef OPAQ_CONFIG_FORECAST_H
#define OPAQ_CONFIG_FORECAST_H

#include <string>
#include <tinyxml.h>
#include <vector>

#include "Component.h"
#include "../Exceptions.h"
#include "../TimeInterval.h"

namespace OPAQ {

namespace Config {

  /**
   * Forecast configuration class
   */
class ForecastStage {
public:
  ForecastStage();
  virtual ~ForecastStage(); 

  /**
   *  Returns the dataprovider for the observed concentration values
   *  Throws OPAQ::NullPointerException
   */
  OPAQ::Config::Component* getValues() const {
    if (values == NULL) throw OPAQ::NullPointerException();
    return values;
  }
  void setValues(OPAQ::Config::Component* values) {
    this->values = values;
  }
  
  // Throws OPAQ::NullPointerException
  OPAQ::Config::Component* getMeteo() const {
    if (meteo == NULL) throw OPAQ::NullPointerException();
    return meteo;
  }
  void setMeteo(OPAQ::Config::Component* meteo) {
    this->meteo = meteo;
  }
  
  // Throws OPAQ::NullPointerException
  OPAQ::Config::Component* getBuffer() const {
    if (buffer == NULL) throw OPAQ::NullPointerException();
    return buffer;
  }
  void setBuffer(OPAQ::Config::Component* buffer) {
    this->buffer = buffer;
  }

  // Throws OPAQ::NullPointerException
  OPAQ::Config::Component* getOutputWriter() const {
    if ( outputWriter == NULL) throw OPAQ::NullPointerException();
    return outputWriter;
  }
  void setOutputWriter(OPAQ::Config::Component* ow ) {
    this->outputWriter = ow;
  }

  // returns a list of models used in the forecast...
  std::vector<OPAQ::Config::Component*> & getModels() { return models; }

  /** Set the requested forecast horizon */
  void setHorizon( const OPAQ::TimeInterval& f ) { fcHor = f; }

  /** Returns the requested (max) forecast horizon for the forecasts */
  OPAQ::TimeInterval& getHorizon() { return fcHor; }

protected:


private:
  // vector of models to run in the forecast
  std::vector<OPAQ::Config::Component *> models;

  // input data provider components
  OPAQ::Config::Component *values;
  OPAQ::Config::Component *meteo;

  // forecast buffer component
  OPAQ::Config::Component *buffer;
  
  // output writer component
  OPAQ::Config::Component *outputWriter;
  
  OPAQ::TimeInterval   fcHor;           //!< requested max forecast horizon
};

} /* namespace Config */

} /* namespace OPAQ */
#endif /* OPAQ_STAGE_H */

