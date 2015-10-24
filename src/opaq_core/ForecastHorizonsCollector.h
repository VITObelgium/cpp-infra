/*
 * ForecastHorizonsCollector.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef FORECASTHORIZONSCOLLECTOR_H_
#define FORECASTHORIZONSCOLLECTOR_H_

#include <data/DataStore.h>

namespace OPAQ {

class ForecastHorizonsCollector: public OPAQ::DataStore {
public:
	ForecastHorizonsCollector();
	virtual ~ForecastHorizonsCollector();

	void setDataStore (DataStore * dataStore) {
		this->dataStore = dataStore;
	}

	std::vector<ForecastHorizon> const & getForecastHorizons () {
		return forecastHorizons;
	}

	// OPAQ::Component method

	virtual void configure (TiXmlElement * configuration) throw (BadConfigurationException) {};

	// OPAQ::DataStore methods

	virtual void setNoData(double noData) {
		dataStore->setNoData(noData);
	}

	virtual double getNoData (){
		return dataStore->getNoData();
	}

	virtual void setBaseTime (const DateTime & baseTime) throw (BadConfigurationException) {
		dataStore->setBaseTime(baseTime);
	}

	virtual void setValues (const std::vector<double> & values,
			const std::vector<ForecastHorizon> & forecastHorizons,
			const std::string & id1, const std::string & id2) {
		// get forecast horizons
		std::vector<ForecastHorizon>::const_iterator it = forecastHorizons.begin();
		while (it != forecastHorizons.end()) {
			const ForecastHorizon * fh = &(*it++);
			if (!contains(fh)) {
				this->forecastHorizons.push_back(*fh);
			}
		}
		// forward call
		dataStore->setValues(values, forecastHorizons, id1, id2);
	}

	virtual void setValues (const std::vector<double> & values, const std::string & id,
				const ForecastHorizon & forecastHorizon = ForecastHorizon(0)) {
		throw RunTimeException("ForecastHorizonsCollector is only meant to be used as an adapter for forecast model output, not for mapping model output");
	}

private:
	DataStore * dataStore;
	std::vector<ForecastHorizon> forecastHorizons;

	bool contains (const ForecastHorizon * fh) {
		std::vector<ForecastHorizon>::iterator it = forecastHorizons.begin();
		while (it != forecastHorizons.end()) {
			if (fh->getSeconds() == (it++)->getSeconds())
				return true;
		}
		return false;
	}


};

} /* namespace OPAQ */
#endif /* FORECASTHORIZONSCOLLECTOR_H_ */
