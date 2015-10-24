/*
 * Hdf5MappingsStore.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef HDF5MAPPINGSSTORE_H_
#define HDF5MAPPINGSSTORE_H_

#include <opaq.h>
#include <H5Cpp.h>

namespace OPAQ {

/**
 * File structure = same as for Hdf5ForecastsStore,
 * only stations are now grid cells
 */
class Hdf5MappingsStore: public OPAQ::DataStore {
public:
	Hdf5MappingsStore();
	virtual ~Hdf5MappingsStore();

	LOGGER_DEC();

	/*
	 * data set that holds the actual data (4D: parameter x cell x date x forecast horizon
	 * . parameter names are stored in another data set
	 * . cell names are stored in another data set
	 * . start date is stored in an attribute, step size = 1 day
	 * . first forecast horizon = 0 days, 2nd one = 1 day, and so on
	 */
	static const std::string DATASET_NAME;
	static const std::string PARAMETERS_DATASET_NAME; // data set that holds the parameter names
	static const std::string CELL_DATASET_NAME; // data set that holds the cell names
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

	virtual void setGridProvider(GridProvider * gridProvider) {
		DataStore::setGridProvider(gridProvider);
		if (_fullyConfigured())
				_openFile();
	}

	virtual void setValues(const std::vector<double> & values,
			const std::vector<ForecastHorizon> & forecastHorizons,
			const std::string & id1, const std::string & id2) {
		throw RunTimeException(
				"Cannot use Hdf5MappingsStore to store forecasts");
	}

	virtual void setValues(const std::vector<double> & values,
			const std::string & parameter,
			const ForecastHorizon & forecastHorizon = ForecastHorizon(0));

private:
	H5::H5File * _h5file;
	bool _configured, _baseTimeSet;
	std::string _filename;
	int _offset;
	DateTime _baseTime;
	DateTime _firstDateInFile;
	double _nodata;
	H5::DataSet _dataSet, _parametersSet, _cellsSet;

	void _closeFile();
	void _openFile();
	bool _fullyConfigured();
	void _openAndCheckFile();
	void _createFile();
	void _checkGridVsFile();

};

} /* namespace OPAQ */
#endif /* HDF5MAPPINGSSTORE_H_ */
