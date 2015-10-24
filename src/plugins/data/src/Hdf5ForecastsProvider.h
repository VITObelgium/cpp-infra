/*
 * Hdf5ForecastsProvider.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef HDF5FORECASTSPROVIDER_H_
#define HDF5FORECASTSPROVIDER_H_

#include <opaq.h>

namespace OPAQ {

class Hdf5ForecastsProvider: public OPAQ::DataProvider {
public:
	Hdf5ForecastsProvider();
	virtual ~Hdf5ForecastsProvider();

	/*
	 * data set that holds the actual data (4D: parameter x station x date x forecast horizon
	 * . parameter names are stored in another data set
	 * . station names are stored in another data set
	 * . start date is stored in an attribute, step size = 1 day
	 * . first forecast horizon = 0 days, 2nd one = 1 day, and so on
	 *
	 * These are the same as those in Hdf5ForecastsStore.h
	 * We repeat them here to avoid creating a dependency of the provider plugin on the store plugin
	 */
	static const std::string DATASET_NAME;
	static const std::string PARAMETERS_DATASET_NAME; // data set that holds the parameter names
	static const std::string STATION_DATASET_NAME; // data set that holds the station names
	static const std::string START_DATE_NAME; // attribute that holds the start date
	static const std::string DIMENSIONS_NAME; // attribute that holds the description of the dims
	static const std::string DIMENSIONS; // the names of the dimensions
	static const std::string DESCRIPTION_NAME; // attribute that holds the description of the data
	static const std::string DESCRIPTION; // the description

	LOGGER_DEC()

	// OPAQ::Component methods

	/**
	 * <filename>/path/to/datafile.h5</filename>
	 */
	virtual void configure(TiXmlElement * configuration)
			throw (BadConfigurationException);

	// OPAQ::DataProvider methods

	virtual void setAQNetworkProvider(AQNetworkProvider * aqNetworkProvider);

	virtual void setBaseTime(const DateTime & baseTime)
			throw (BadConfigurationException);

	virtual TimeInterval getTimeResolution();

	virtual std::pair<const TimeInterval, const TimeInterval> getRange();

	virtual std::pair<const TimeInterval, const TimeInterval> getRange(
			const ForecastHorizon & forecastHorizon);

	virtual unsigned int size();

	virtual double getNoData();

	/*
	 * id is ignored, returns the result of getNoData()
	 */
	virtual double getNoData(const std::string & id);

	/*
	 * returns the values for forecast horizon = 0
	 */
	virtual std::vector<double> getValues(const TimeInterval & beginOffset,
			const TimeInterval & endOffset, const std::string & parameter,
			const std::string & station);

	virtual std::vector<double> getValues(const TimeInterval & beginOffset,
			const TimeInterval & endOffset, const std::string & parameter,
			const std::string & station,
			const ForecastHorizon & forecastHorizon);

	virtual std::vector<double> getValues(const std::string & parameter,
			const TimeInterval & offset = TimeInterval(0),
			const ForecastHorizon & forecastHorizon = ForecastHorizon(0));

private:
	H5::H5File * _h5file;
	H5::DataSet _dataSet, _parametersSet, _stationsSet;
	// first base time in data file
	DateTime _startDate;
	// base time used in the application
	DateTime _baseTime;
	AQNetworkProvider * _aqNetworkProvider;
	bool _configured, _baseTimeSet;
	std::string _filename;

	void _checkFullyConfigured() throw (NotConfiguredException);
	void _openFile(const std::string & filename)
			throw (BadConfigurationException);

};

} /* namespace OPAQ */
#endif /* HDF5FORECASTSPROVIDER_H_ */
