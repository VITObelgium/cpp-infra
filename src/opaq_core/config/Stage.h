/*
 * Plugin.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#ifndef OPAQ_STAGE_H
#define OPAQ_STAGE_H

#include <string>
#include <tinyxml.h>
#include <vector>

#include "Component.h"
#include "../Exceptions.h"

namespace OPAQ {

namespace Config {

class Stage {
public:
	Stage();
	virtual ~Stage() = 0; // make Stage an abstract class

	bool isEnsemble() const {
		return ensemble;
	}

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

	OPAQ::Config::Component* getHistoricalForecasts() const throw (OPAQ::NullPointerException) {
		if (historicalForecasts == NULL) throw OPAQ::NullPointerException();
		return historicalForecasts;
	}
	void setHistoricalForecasts(OPAQ::Config::Component* historicalForecasts) {
		this->historicalForecasts = historicalForecasts;
	}

	OPAQ::Config::Component* getOutput() const throw (OPAQ::NullPointerException) {
		if (output == NULL) throw OPAQ::NullPointerException();
		return output;
	}
	void setOutput(OPAQ::Config::Component* output) {
		this->output = output;
	}

protected:
	bool ensemble;

private:
	// input data provider components
	OPAQ::Config::Component *values;
	OPAQ::Config::Component *meteo;
	OPAQ::Config::Component *historicalForecasts;
	// output data provider component
	OPAQ::Config::Component *output;
};

} /* namespace Config */

} /* namespace OPAQ */
#endif /* OPAQ_STAGE_H */

