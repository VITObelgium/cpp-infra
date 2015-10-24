/*
 * Hdf5ForecastsStore.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "Hdf5ForecastsStore.h"

namespace OPAQ {

Hdf5ForecastsStore::Hdf5ForecastsStore() {
	// tell the hdf5 lib not to print error messages: we will handle them properly ourselves
	H5::Exception::dontPrint();
	_h5file = NULL;
	_noData = -9999;
	_configured = false;
	_baseTimeSet = false;
}

Hdf5ForecastsStore::~Hdf5ForecastsStore() {
	_closeFile();
}

LOGGER_DEF(Hdf5ForecastsStore);

const std::string Hdf5ForecastsStore::DATASET_NAME("forecasted_values");
const std::string Hdf5ForecastsStore::PARAMETERS_DATASET_NAME("parameters");
const std::string Hdf5ForecastsStore::STATION_DATASET_NAME("stations");
const std::string Hdf5ForecastsStore::START_DATE_NAME("start_date");
const std::string Hdf5ForecastsStore::DIMENSIONS_NAME("dimensions");
const std::string Hdf5ForecastsStore::DIMENSIONS(
		"parameter x station x base time x forecast horizon");
const std::string Hdf5ForecastsStore::DESCRIPTION_NAME("description");
const std::string Hdf5ForecastsStore::DESCRIPTION("OPAQ forecasts");

void Hdf5ForecastsStore::configure(TiXmlElement * configuration)
		throw (BadConfigurationException) {

	if (_configured) {
		_closeFile();
	}

	// 1. parse filename
	TiXmlElement * fileEl = configuration->FirstChildElement("filename");
	if (!fileEl)
		throw BadConfigurationException("filename element not found");
	_filename = fileEl->GetText();

	// 2. parse start date
	TiXmlElement * offsetEl = configuration->FirstChildElement("offset");
	if (!offsetEl)
		throw BadConfigurationException("offset element not found");
	_offset = atoi(offsetEl->GetText());
	if (_baseTimeSet)
		_calcStartDateAndOpenFile();

	_configured = true;

}

void Hdf5ForecastsStore::setNoData(double noData) {
	this->_noData = noData;
}

double Hdf5ForecastsStore::getNoData() {
	return _noData;
}

void Hdf5ForecastsStore::setBaseTime(const DateTime & baseTime)
		throw (BadConfigurationException) {
	this->_baseTime = DateTimeTools::floor(baseTime, DateTimeTools::FIELD_DAY);
	if (_configured)
		_calcStartDateAndOpenFile();

	_baseTimeSet = true;
}

void Hdf5ForecastsStore::setValues(const std::vector<double> & values,
		const std::vector<ForecastHorizon> & forecastHorizons,
		const std::string & parameter, const std::string & station) {
	_checkFullyConfigured();

	if (values.size() == 0)
		return; // do nothing if no values are given

	// 1. validate data
	if (values.size() != forecastHorizons.size())
		throw RunTimeException(
				"number of values != number of forecastHorizons");
	std::vector<ForecastHorizon>::const_iterator it = forecastHorizons.begin();
	long previousHours = 0;
	while (it != forecastHorizons.end()) {
		long hours = it->getHours();
		if (hours % 24 != 0)
			throw RunTimeException(
					"Not all forecast horizons lie on exact day boundaries");
		if (it != forecastHorizons.begin() && hours - previousHours != 24) {
			throw RunTimeException(
					"Forecast horizons must be consecutive days (24 hours)");
		}
		previousHours = hours;
		it++;
	}

	// 2. get parameter, station, date, and forecast horizon indices
	unsigned int parameterIndex = Hdf5Tools::getIndexInStringDataSet(
			_parametersSet, parameter, true);
	unsigned int stationIndex = Hdf5Tools::getIndexInStringDataSet(_stationsSet,
			station, true);
	unsigned int dateIndex = TimeInterval(_startDate, _baseTime).getDays();
	unsigned int fhIndex = forecastHorizons.begin()->getHours() / 24;

	// 3. store data
	// a. get data space
	H5::DataSpace space = _dataSet.getSpace();
	// b. get current size
	hsize_t size[4];
	space.getSimpleExtentDims(size, NULL);
	bool extend = false;
	if (parameterIndex >= size[0]) {
		size[0] = parameterIndex + 1;
		extend = true;
	}
	if (stationIndex >= size[1]) {
		size[1] = stationIndex + 1;
		extend = true;
	}
	if (dateIndex >= size[2]) {
		size[2] = dateIndex + 1;
		extend = true;
	}
	if (fhIndex + forecastHorizons.size() > size[3]) {
		size[3] = fhIndex + forecastHorizons.size();
		extend = true;
	}
	// c. extend data set if required
	if (extend) {
		_dataSet.extend(size);
		space.close();
		space = _dataSet.getSpace();
	}
	// d. select data set hyperslab
	hsize_t count[4] = { 1, 1, 1, forecastHorizons.size() }; // TODO: set chunk size > 1 in fh dim?
	hsize_t offset[4] = { parameterIndex, stationIndex, dateIndex, fhIndex };
	space.selectHyperslab(H5S_SELECT_SET, count, offset);
	// e. write data to the hyperslab
	H5::DataSpace writeMemSpace(4, count);
	double fileNoData;
	_dataSet.getCreatePlist().getFillValue(H5::PredType::NATIVE_DOUBLE,
			&fileNoData);
	if (_noData == fileNoData) {
		// nodata placeholder in file equals the one used in the values vector
		_dataSet.write(&values[0], H5::PredType::NATIVE_DOUBLE, writeMemSpace,
				space);
	} else {
		// nodata placeholder in file is different from the one used in the values vector
		double newValues[count[3]];
		for (unsigned int i = 0; i < values.size(); i++) {
			newValues[i] = values[i] == _noData ? fileNoData : values[i];
		}
		_dataSet.write(&newValues[0], H5::PredType::NATIVE_DOUBLE,
				writeMemSpace, space);
	}
	space.close();
	_dataSet.flush(H5F_SCOPE_GLOBAL);
}

void Hdf5ForecastsStore::setValues(const std::vector<double> & values,
		const std::string & id,
		const ForecastHorizon & forecastHorizon) {
	throw RunTimeException("Cannot use Hdf5ForecastsStore to store mappings.");
}

void Hdf5ForecastsStore::_closeFile() {
	if (_h5file != NULL) {
		_dataSet.close();
		_parametersSet.close();
		_stationsSet.close();
		_h5file->close();
		delete _h5file;
	}
}

void Hdf5ForecastsStore::_checkFullyConfigured() {
	if (!_configured || !_baseTimeSet)
		throw NotConfiguredException("Not fully configured");
}

void Hdf5ForecastsStore::_calcStartDateAndOpenFile()
		throw (BadConfigurationException) {
	if (_h5file == NULL) { // don't open a new file if another base time is set
		_startDate = _baseTime;
		_startDate.addDays(_offset);
		if (_startDate > _baseTime)
			throw BadConfigurationException(
					"start date cannot be after base time");

		if (FileTools::exists(_filename)) {
			_openFile(_filename);
		} else {
			_createFile(_filename);
		}
	}
}

void Hdf5ForecastsStore::_createFile(const std::string & filename) {
	// 1. main data set
	// create data space
	hsize_t dims[4] = { 0, 0, 0, 0 };
	hsize_t maxdims[4] = { H5S_UNLIMITED, H5S_UNLIMITED, H5S_UNLIMITED,
			H5S_UNLIMITED };
	H5::DataSpace dataSpace(4, dims, maxdims);
	// create file
	try {
		_h5file = new H5::H5File(filename, H5F_ACC_TRUNC);
	} catch (H5::FileIException & e) {
		std::stringstream ss;
		ss << "Failed to create file " << filename
				<< " (disk full? no write permissions?)";
		throw BadConfigurationException(ss.str());
	}
	// enable chunking & set chunk size
	H5::DSetCreatPropList cparms;
	hsize_t chunks[4] = { 1, 10, 10, 1 };
	cparms.setChunk(4, chunks);
	// set nodata value
	cparms.setFillValue(H5::PredType::NATIVE_DOUBLE, &_noData);
	// create data set
	_dataSet = _h5file->createDataSet(DATASET_NAME, H5::PredType::NATIVE_DOUBLE,
			dataSpace, cparms);
	// add data set attributes
	std::stringstream ss;
	ss << _startDate;
	Hdf5Tools::createStringAttribute(_dataSet, START_DATE_NAME, ss.str().substr(0, 10));
	Hdf5Tools::createStringAttribute(_dataSet, DIMENSIONS_NAME, DIMENSIONS);
	Hdf5Tools::createStringAttribute(_dataSet, DESCRIPTION_NAME, DESCRIPTION);

	// 2. meta data: parameters & stations
	hsize_t dims2[1] = { 0 };
	hsize_t maxdims2[1] = { H5S_UNLIMITED };
	H5::DataSpace dataSpace2(1, dims2, maxdims2);
	H5::DSetCreatPropList cparms2;
	hsize_t chunks2[1] = { 10 };
	cparms2.setChunk(1, chunks2);
	std::string noData("n/a");
	cparms2.setFillValue(Hdf5Tools::stringType, &noData);
	_parametersSet = _h5file->createDataSet(PARAMETERS_DATASET_NAME,
			Hdf5Tools::stringType, dataSpace2, cparms2);
	_stationsSet = _h5file->createDataSet(STATION_DATASET_NAME, Hdf5Tools::stringType,
			dataSpace2, cparms2);
}

void Hdf5ForecastsStore::_openFile(const std::string &filename)
		throw (BadConfigurationException) {
	// 1. open file
	try {
		_h5file = new H5::H5File(filename, H5F_ACC_RDWR);
	} catch (H5::FileIException & e) {
		std::stringstream ss;
		ss << "Failed to open file " << filename
				<< " (is it a valid HDF5 file?)";
		throw BadConfigurationException(ss.str());
	}

	// 2. check file format
	try {
		_dataSet = _h5file->openDataSet(DATASET_NAME);
		_parametersSet = _h5file->openDataSet(PARAMETERS_DATASET_NAME);
		_stationsSet = _h5file->openDataSet(STATION_DATASET_NAME);
	} catch (H5::FileIException & e) {
		std::stringstream ss;
		ss << "File " << filename << " does not contain forecasts data";
		throw BadConfigurationException(ss.str());
	}

	try {
		std::string dateStr = Hdf5Tools::readStringAttribute(_dataSet,
				START_DATE_NAME);
		DateTime fromFile = DateTimeTools::parseDate(dateStr);
		if (fromFile > _startDate) {
			std::stringstream ss;
			ss << "start date is before the first date in file " << filename;
			throw BadConfigurationException(ss.str());
		}
		_startDate = fromFile;
	} catch (H5::AttributeIException & e) {
		std::stringstream ss;
		ss << "Failed to get start date from file " << filename
				<< " (file corrupted?)";
		throw BadConfigurationException(ss.str());
	} catch (ParseException & e) {
		std::stringstream ss;
		ss << "Failed to parse date in start_date attribute in file "
				<< filename << " (file corrupted?)";
		throw BadConfigurationException(ss.str());
	}

	// 3. get nodata placeholder
	_dataSet.getCreatePlist().getFillValue(H5::PredType::NATIVE_DOUBLE,
			&_noData);

	// TODO further checks if required: do dimensions match etc
}

} /* namespace OPAQ */

OPAQ_REGISTER_PLUGIN(OPAQ::Hdf5ForecastsStore);
