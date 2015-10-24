/*
 * ECMWFMeteoProvider.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef ECMWFMETEOPROVIDER_H_
#define ECMWFMETEOPROVIDER_H_

#include <opaq.h>

namespace OPAQ {

class ECMWFMeteoProvider: public OPAQ::DataProvider {
public:
	ECMWFMeteoProvider();
	virtual ~ECMWFMeteoProvider();

	LOGGER_DEC();

	static const std::string METEO_PLACEHOLDER;
	static const std::string PARAMETER_PLACEHOLDER;

	// OPAQ::Component methods

	/**
	 * ECMWFMeteoProvider configuration:
	 * <!-- range to buffer, offsets relative to base time
	 *   the unit of the values is 'number of meteo steps'
	 *   a meteo step is 6 hours long, which means
	 *   a value of 4 here means 1 day,
	 *   a value of 12 means 3 days, etc. -->
	 * <range>
	 * 	<begin_offset>-12</begin_offset>
	 * 	<end_offset>8</end_offset>
	 * </range>
	 * <parameters>
	 *  <parameter id="some_id" nodata="-999" />
	 *  <parameter id="some_other_id" nodata="0" />
	 * </parameters>
	 * <file_pattern>/full/path/to/data/file/name_of_file_%METEO%_%PARAMETER%.gz</file_pattern>
	 */
	virtual void configure(TiXmlElement * configuration)
			throw (BadConfigurationException);

	// OPAQ::DataProvider methods

	/**
	 * Ignored
	 */
	virtual void setAQNetworkProvider(AQNetworkProvider * aqNetworkProvider);

	virtual void setBaseTime (const DateTime & baseTime) throw (BadConfigurationException);

	virtual TimeInterval getTimeResolution();

	virtual std::pair<const TimeInterval, const TimeInterval> getRange();

	virtual std::pair<const TimeInterval, const TimeInterval> getRange(const ForecastHorizon & forecastHorizon);

	virtual unsigned int size();

	/**
	 * throws an exception: use getNoData(const std::string &) instead
	 */
	virtual double getNoData();

	virtual double getNoData(const std::string & parameterId);

	virtual std::vector<double> getValues(const TimeInterval & beginOffset,
			const TimeInterval & endOffset, const std::string & meteoId,
			const std::string & parameterId);

	/**
	 * The forecastHorizon parameter will be ignored.
	 */
	virtual std::vector<double> getValues(const TimeInterval & beginOffset,
			const TimeInterval & endOffset, const std::string & meteoId,
			const std::string & parameterId,
			const ForecastHorizon & forecastHorizon);

	virtual std::vector<double> getValues(const std::string & parameterId,
				const TimeInterval & offset = TimeInterval(0),
				const ForecastHorizon & forecastHorizon = ForecastHorizon(0)) {
		throw RunTimeException("ECMWFMeteoProvider provides meteo values, which cannot be requested 'by AQ network'");
		// TODO: implement this anyway
	}

private:
	TimeInterval _timeResolution;
	DateTime _begin, _end, _baseTime;
	long _beginOffset, _endOffset;
	std::string _pattern;
	std::map<std::string, double> _nodata;
	double _defaultNodata;
	std::map<std::string, std::vector<double> > _emptyBuffer;
	std::map<std::string, std::map<std::string, std::vector<double> > > _buffer;
	bool _configured, _baseTimeSet;

	void _checkFullyConfigured () throw (NotConfiguredException);

	void _calculateBeginEnd();

	/*
	 * Change the given datetime into the nearest "6"-hour <= the given datetime
	 */
	void _floor6 (DateTime & datetime);
	/*
	 * Change the given datetime into the nearest "6"-hour >= the given datetime
	 */
	void _ceil6 (DateTime & datetime);

	const std::vector<double> & _getValues(const std::string & meteoId, const std::string & parameterId);

	void _readFile (const std::string & meteoId, const std::string & parameterId);

	std::vector<double> * _getOrInitValues(const std::string & meteoId, const std::string &parameterId);

	std::vector<double> & _getEmpty(const std::string & parameterId);
};

} /* namespace OPAQ */
#endif /* ECMWFMETEOPROVIDER_H_ */
