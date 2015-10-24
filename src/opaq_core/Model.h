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
#include "data/DataStore.h"
#include "data/GridProvider.h"
#include "Pollutant.h"
#include "AQNetworkProvider.h"

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
	virtual void setForecastHorizon(ForecastHorizon * forecastHorizon) {
		this->forecastHorizon = forecastHorizon;
	}
	virtual void setAQNetworkProvider(
			AQNetworkProvider * aqNetworkProvider) {
		this->aqNetworkProvider = aqNetworkProvider;
	}
	virtual void setGridProvider(GridProvider * gridProvider) {
		this->gridProvider = gridProvider;
	}
	virtual void setInputProvider(DataProvider * input) {
		this->input = input;
	}
	virtual void setMeteoProvider(DataProvider * meteo) {
		this->meteo = meteo;
	}
	virtual void setHistoricalForecastsProvider(
			DataProvider * historicalForecasts) {
		this->historicalForecasts = historicalForecasts;
	}
	virtual void setOutputStore(DataStore * output) {
		this->output = output;
	}

	virtual const DateTime & getBaseTime() {
		return baseTime;
	}
	virtual const Pollutant & getPollutant() {
		return pollutant;
	}
	virtual ForecastHorizon * getForecastHorizon() throw (NullPointerException) {
		if (forecastHorizon == NULL) throw NullPointerException();
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
	virtual DataProvider * getMeteoProvider() throw (NullPointerException) {
		if (meteo == NULL) throw NullPointerException();
		return meteo;
	}
	virtual DataProvider * getHistoricalForecastsProvider() throw (NullPointerException) {
		if (historicalForecasts == NULL) throw NullPointerException();
		return historicalForecasts;
	}
	virtual DataStore * getOutputStore() throw (NullPointerException) {
		if (output == NULL) throw NullPointerException();
		return output;
	}

	virtual void run() = 0;

private:
	DateTime baseTime;
	Pollutant pollutant;
	ForecastHorizon * forecastHorizon;
	AQNetworkProvider * aqNetworkProvider;
	GridProvider * gridProvider;
	DataProvider * input;
	DataProvider * meteo;
	DataProvider * historicalForecasts;
	DataStore * output;

};

} /* namespace opaq */
#endif /* OPAQ_MODEL_H_ */
