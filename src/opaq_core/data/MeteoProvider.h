/*
 * MeteoProvider.h
 *
 *  Created on: Dec 18, 2015
 *      Author: bino
 */

#ifndef SRC_OPAQ_CORE_DATA_METEOPROVIDER_H_
#define SRC_OPAQ_CORE_DATA_METEOPROVIDER_H_

#include <vector>
#include "../Component.h"
#include "../DateTime.h"
#include "../TimeSeries.h"

namespace OPAQ {

class MeteoProvider: public OPAQ::Component {
public:
	MeteoProvider();
	virtual ~MeteoProvider();

	/**
	 * Get the time resolution of the provided data in the meteo data provider
	 */
	virtual std::chrono::hours getTimeResolution() = 0;

	/**
	 * Get the nodata placeholder for the given parameter
	 * For example: for meteo data, the no data placeholder might be different for each
	 * parameter.
	 */
	virtual double getNoData(const std::string & id) = 0;

	/**
	 * Return the values between t1 and t2 including the boundaries !
	 */
	virtual OPAQ::TimeSeries<double> getValues( const DateTime & t1,
				                                const DateTime & t2,
												const std::string& meteoId,
												const std::string& paramId ) = 0;

	/**
	 * Set basetime for the meteo provider
	 */
	void setBaseTime( const OPAQ::DateTime& t ) { _baseTime = t; }
	const OPAQ::DateTime& getBaseTime(){ return _baseTime; }

protected:
	OPAQ::DateTime _baseTime;
};

} /* namespace OPAQ */

#endif /* SRC_OPAQ_CORE_DATA_METEOPROVIDER_H_ */
