/*
 * DataStore.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef OPAQ_DATA_STORE_H_
#define OPAQ_DATA_STORE_H_

#include <vector>

#include "Component.h"

#include "../ForecastHorizon.h"
#include "../Grid.h"
#include "../data/GridProvider.h"

namespace OPAQ {

class DataStore: public OPAQ::Component {
public:
	DataStore();
	virtual ~DataStore();

	/**
	 * Set the nodata placeholder to use for incoming values,
	 * alternatively you can also use the current nodata placeholder
	 * as returned by getNoData()
	 */
	virtual void setNoData(double noData) = 0;

	/**
	 * Get the nodata placeholder currently used, alternatively you
	 * can set your own using setNoData(double)
	 */
	virtual double getNoData() = 0;

	/**
	 * Set the base time
	 */
	virtual void setBaseTime(const DateTime & baseTime)
			throw (BadConfigurationException) = 0;

	virtual void setGridProvider(GridProvider * gridProvider) {
		_gridProvider = gridProvider;
	}

	virtual GridProvider * getGridProvider() {
		if (_gridProvider == NULL)
			throw NullPointerException("Grid provider not set");
		return _gridProvider;
	}

	/**
	 * Store the given values with the given forecast horizons for given
	 * ids.
	 *
	 * Only forecasted values are stored.
	 * For output in an AQNetwork, these ids are forecasted parameter & station,
	 *
	 * This method is typically used by a forecast model
	 */
	virtual void setValues(const std::vector<double> & values,
			const std::vector<ForecastHorizon> & forecastHorizons,
			const std::string & id1, const std::string & id2) = 0;

	// TODO: add setters for data mergers (model id might be needed to allow the merger to identify the model)

	/**
	 * Store the given values, for the given id,  with the given forecast horizon
	 * into the given grid (via setGridProvider)
	 *
	 * the id is a pollutant when mapping observations, or a forecasted parameter when mapping forecasts.
	 * values contains a value for each cell in the grid
	 *
	 * This method is typically used by a mapping model
	 */
	virtual void setValues(const std::vector<double> & values,
			const std::string & id, const ForecastHorizon & forecastHorizon =
					ForecastHorizon(0)) = 0;

private:
	GridProvider * _gridProvider;

};

} /* namespace OPAQ */
#endif /* OPAQ_DATA_STORE_H_ */
