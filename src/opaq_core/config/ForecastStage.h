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
#include "../ForecastHorizon.h"

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
   */
  OPAQ::Config::Component* getValues() const throw (OPAQ::NullPointerException) {
    if (values == NULL) throw OPAQ::NullPointerException();
    return values;
  }
  void setValues(OPAQ::Config::Component* values) {
    this->values = values;
  }
  
  OPAQ::Config::Component* getMeteo() const throw (OPAQ::NullPointerException) {
    if (meteo == NULL) throw OPAQ::NullPointerException();
    return meteo;
  }
  void setMeteo(OPAQ::Config::Component* meteo) {
    this->meteo = meteo;
  }
  
  OPAQ::Config::Component* getBuffer() const throw (OPAQ::NullPointerException) {
    if (buffer == NULL) throw OPAQ::NullPointerException();
    return buffer;
  }
  void setBuffer(OPAQ::Config::Component* buffer) {
    this->buffer = buffer;
  }

  OPAQ::Config::Component* getOutputWriter() const throw (OPAQ::NullPointerException) {
    if ( outputWriter == NULL) throw OPAQ::NullPointerException();
    return outputWriter;
  }
  void setOutputWriter(OPAQ::Config::Component* ow ) {
    this->outputWriter = ow;
  }


  OPAQ::Config::Component* getMerger() const { return merger; }
  void setMerger(OPAQ::Config::Component* merger) { this->merger = merger; }

  // returns a list of models used in the forecast...
  std::vector<OPAQ::Config::Component*> & getModels() { return models; }

  /** Set the requested forecast horizon */
  void setHorizon( int ndays ) {
    OPAQ::ForecastHorizon f( 24*ndays );
    fcHor = f;
  }

  /** Returns the requested (max) forecast horizon for the forecasts */
  OPAQ::ForecastHorizon & getHorizon() { return fcHor; }

protected:


private:
  // vector of models to run in the forecast
  std::vector<OPAQ::Config::Component *> models;

  OPAQ::Config::Component * merger;

  // input data provider components
  OPAQ::Config::Component *values;
  OPAQ::Config::Component *meteo;

  // forecast buffer component
  OPAQ::Config::Component *buffer;
  
  // output writer component
  OPAQ::Config::Component *outputWriter;
  
  OPAQ::ForecastHorizon    fcHor;           //!< requested max forecast horizon
};

} /* namespace Config */

} /* namespace OPAQ */
#endif /* OPAQ_STAGE_H */

