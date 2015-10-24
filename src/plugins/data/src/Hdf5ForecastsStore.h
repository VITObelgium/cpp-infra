/*
 * Hdf5ForecastsStore.h
 *
 *	Stores forecasts per (forecast)parameter, station, forecast horizon
 *	1 value per day
 *	forecast horizon step = 1 day
 *	default no data = -9999
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef HDF5FORECASTSSTORE_H_
#define HDF5FORECASTSSTORE_H_

#include <opaq.h>
#include <H5Cpp.h>

namespace OPAQ {

class Hdf5ForecastsStore: public OPAQ::DataStore {
public:
	Hdf5ForecastsStore();
	virtual ~Hdf5ForecastsStore();

	LOGGER_DEC()
	;

	/*
	 * data set that holds the actual data (4D: parameter x station x date x forecast horizon
	 * . parameter names are stored in another data set
	 * . station names are stored in another data set
	 * . start date is stored in an attribute, step size = 1 day
	 * . first forecast horizon = 0 days, 2nd one = 1 day, and so on
	 */
	static const std::string DATASET_NAME;
	static const std::string PARAMETERS_DATASET_NAME; // data set that holds the parameter names
	static const std::string STATION_DATASET_NAME; // data set that holds the station names
	static const std::string START_DATE_NAME; // attribute that holds the start date
	static const std::string DIMENSIONS_NAME; // attribute that holds the description of the dims
	static const std::string DIMENSIONS; // the names of the dimensions
	static const std::string DESCRIPTION_NAME; // attribute that holds the description of the data
	static const std::string DESCRIPTION; // the description

	// OPAQ::Component methods

	/**
	 * <!-- created if it does not exist, appended if it does -->
	 * <filename>/path/to/datafile.h5</filename>
	 * <!-- offset in days relative to the base time indicating
	 *   the earliest day for which data will be stored
	 *   if a new file is created, this will be the first date in the file
	 *   when an existing file is opened, this cannot point to a date
	 *   before the first date in the file
	 *   for example: if base time is 2014/01/10 and offset = -10,
	 *    the first date stored is 2013/12/31
	 * <offset>-10</offset>
	 */
	virtual void configure(TiXmlElement * configuration)
			throw (BadConfigurationException);

	// OPAQ::DataStore methods

	virtual void setNoData(double noData);

	virtual double getNoData();

	virtual void setBaseTime(const DateTime & baseTime)
			throw (BadConfigurationException);

	/**
	 * throws Hdf5ForecastsStoreRunTimeException on runtime errors:
	 * . forecast horizons that don't lie on exact day boundaries
	 * . number of values != number of forecast horizons
	 */
	virtual void setValues(const std::vector<double> & values,
			const std::vector<ForecastHorizon> & forecastHorizons,
			const std::string & parameter, const std::string & station);

	virtual void setValues(const std::vector<double> & values,
			const std::string & id,
			const ForecastHorizon & forecastHorizon = ForecastHorizon(0));

private:
	H5::H5File * _h5file;
	std::string _filename;
	H5::DataSet _dataSet, _parametersSet, _stationsSet;
	double _noData;
	int _offset;
	DateTime _startDate, _baseTime;
	bool _configured, _baseTimeSet;

	void _closeFile();

	void _checkFullyConfigured();

	void _calcStartDateAndOpenFile() throw (BadConfigurationException);

	void _createFile(const std::string & filename);

	void _openFile(const std::string &filename)
			throw (BadConfigurationException);

//	int _getIndexInStringDataSet (H5::DataSet & dataSet, const std::string &parameter);
//	unsigned int _getStringDataSetSize (H5::DataSet & dataSet);
//	void _readStringData (char ** buffer, H5::DataSet & dataSet);
//	void _addToStringDataSet (H5::DataSet & dataSet, const std::string & value);
};

} /* namespace OPAQ */
#endif /* HDF5FORECASTSSTORE_H_ */
