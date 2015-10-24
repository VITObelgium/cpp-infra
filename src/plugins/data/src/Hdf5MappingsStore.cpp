/*
 * Hdf5MappingsStore.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "Hdf5MappingsStore.h"

namespace OPAQ {

LOGGER_DEF(OPAQ::Hdf5MappingsStore)

const std::string Hdf5MappingsStore::DATASET_NAME("mapped_values");
const std::string Hdf5MappingsStore::PARAMETERS_DATASET_NAME("parameters");
const std::string Hdf5MappingsStore::CELL_DATASET_NAME("cells");
const std::string Hdf5MappingsStore::START_DATE_NAME("start_date");
const std::string Hdf5MappingsStore::DIMENSIONS_NAME("dimensions");
const std::string Hdf5MappingsStore::DIMENSIONS(
		"parameter x cell x base time x forecast horizon");
const std::string Hdf5MappingsStore::DESCRIPTION_NAME("description");
const std::string Hdf5MappingsStore::DESCRIPTION("OPAQ mappings");

Hdf5MappingsStore::Hdf5MappingsStore() {
	// tell the hdf5 lib not to print error messages: we will handle them properly ourselves
	H5::Exception::dontPrint();
	_h5file = NULL;
	_configured = false;
	_baseTimeSet = false;
	_nodata = -9999;
}

Hdf5MappingsStore::~Hdf5MappingsStore() {
	_closeFile();
}

void Hdf5MappingsStore::configure(TiXmlElement * configuration)
		throw (BadConfigurationException) {
	if (_configured)
		_closeFile();

	// 1. parse filename
	TiXmlElement * fileEl = configuration->FirstChildElement("filename");
	if (!fileEl)
		throw BadConfigurationException("filename element not found");
	_filename = fileEl->GetText();

	// 2. parse offset
	TiXmlElement * offsetEl = configuration->FirstChildElement("offset");
	if (!offsetEl)
		throw BadConfigurationException("offset element not found");
	_offset = atoi(offsetEl->GetText());

	_configured = true;

	if (_fullyConfigured())
		_openFile();
}

void Hdf5MappingsStore::setNoData(double noData) {
	_nodata = noData;
}

double Hdf5MappingsStore::getNoData() {
	return _nodata;
}

void Hdf5MappingsStore::setBaseTime(const DateTime & baseTime)
		throw (BadConfigurationException) {
	_baseTime = DateTimeTools::floor(baseTime, DateTimeTools::FIELD_DAY);
	_baseTimeSet = true;
	if (_fullyConfigured())
		_openFile();
}

void Hdf5MappingsStore::setValues(const std::vector<double> & values,
		const std::string & parameter,
		const ForecastHorizon & forecastHorizon) {

	// check if all is ok to proceed
	if (!_fullyConfigured())
		throw RunTimeException(
				"Hdf5MappingsStore not fully configured, cannot store values yet");

	_checkGridVsFile();

	if (values.size() != Hdf5Tools::getDataSetSize(_cellsSet))
		throw RunTimeException("number of values doesn't match the number of grid cells: cannot store values");

	if (forecastHorizon.getHours() % 24 != 0)
		throw RunTimeException("forecast horizon does not lie on exact day boundary");

	// then proceed

	// 1. get parameter, date and forecast horizon index
	unsigned int parameterIndex = Hdf5Tools::getIndexInStringDataSet(_parametersSet, parameter, true);
	unsigned int dateIndex = TimeInterval(_firstDateInFile, _baseTime).getDays();
	unsigned int fhIndex = forecastHorizon.getHours() / 24;

	// 2. store data
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
	if (size[1] != values.size()) {
		size[1] = values.size();
		extend = true;
	}
	if (dateIndex >= size[2]) {
		size[2] = dateIndex + 1;
		extend = true;
	}
	if (fhIndex >= size[3]) {
		size[3] = fhIndex + 1;
		extend = true;
	}
	// c. extend data set if required
	if (extend) {
		_dataSet.extend(size);
		space.close();
		space = _dataSet.getSpace();
	}
	// d. select data set hyperslab
	hsize_t count[4] = { 1, size[1], 1, 1 };
	hsize_t offset[4] = { parameterIndex, 0, dateIndex, fhIndex };
	space.selectHyperslab(H5S_SELECT_SET, count, offset);
	// e. write data to the hyperslab
	H5::DataSpace writeMemSpace(4, count);
	double fileNoData;
	_dataSet.getCreatePlist().getFillValue(H5::PredType::NATIVE_DOUBLE,
			&fileNoData);
	if (_nodata == fileNoData) {
		// nodata placeholder in file equals the one used in the values vector
		_dataSet.write(&values[0], H5::PredType::NATIVE_DOUBLE, writeMemSpace,
				space);
	} else {
		// nodata placeholder in file is different from the one used in the values vector
		double newValues[count[3]];
		for (unsigned int i = 0; i < values.size(); i++) {
			newValues[i] = values[i] == _nodata ? fileNoData : values[i];
		}
		_dataSet.write(&newValues[0], H5::PredType::NATIVE_DOUBLE,
				writeMemSpace, space);
	}
	space.close();
	_dataSet.flush(H5F_SCOPE_GLOBAL);
	// TODO: test this !
}

void Hdf5MappingsStore::_closeFile() {
	if (_h5file != NULL) {
		_dataSet.close();
		_parametersSet.close();
		_cellsSet.close();
		_h5file->close();
		delete _h5file;
	}
}

void Hdf5MappingsStore::_openFile() {
	if (_h5file == NULL) { // only open file if it hasn't been opened yet
		_firstDateInFile = _baseTime;
		_firstDateInFile.addDays(_offset);
		if (_firstDateInFile > _baseTime)
			throw BadConfigurationException(
					"first date in file cannot be after base time");
		if (FileTools::exists(_filename)) {
			_openAndCheckFile();
		} else {
			_createFile();
		}
	}
}

bool Hdf5MappingsStore::_fullyConfigured() {
	try {
		getGridProvider();
	} catch (NullPointerException & e) {
		return false;	// no grid provider set
	}
	return _configured && _baseTimeSet;
}

void Hdf5MappingsStore::_openAndCheckFile() {
	// 1. open file
	try {
		_h5file = new H5::H5File(_filename, H5F_ACC_RDWR);
	} catch (H5::FileIException & e) {
		std::stringstream ss;
		ss << "Failed to open file " << _filename
				<< " (is it a valid HDF5 file?)";
		throw BadConfigurationException(ss.str());
	}

	// 2. check file format
	try {
		_dataSet = _h5file->openDataSet(DATASET_NAME);
		_parametersSet = _h5file->openDataSet(PARAMETERS_DATASET_NAME);
		_cellsSet = _h5file->openDataSet(CELL_DATASET_NAME);
	} catch (H5::FileIException & e) {
		std::stringstream ss;
		ss << "File " << _filename << " does not contain mappings data";
		throw BadConfigurationException(ss.str());
	}

	_checkGridVsFile();

	try {
		std::string dateStr = Hdf5Tools::readStringAttribute(_dataSet,
				START_DATE_NAME);
		DateTime fromFile = DateTimeTools::parseDate(dateStr);
		if (fromFile > _firstDateInFile) {
			std::stringstream ss;
			ss
					<< "given first date (base time + offset) is before the first date in file "
					<< _filename;
			throw BadConfigurationException(ss.str());
		}
		_firstDateInFile = fromFile;
	} catch (H5::AttributeIException & e) {
		std::stringstream ss;
		ss << "Failed to get first date from file " << _filename
				<< " (file corrupted?)";
		throw BadConfigurationException(ss.str());
	} catch (ParseException & e) {
		std::stringstream ss;
		ss << "Failed to parse date in start_date attribute in file "
				<< _filename << " (file corrupted?)";
		throw BadConfigurationException(ss.str());
	}

	// 3. get nodata placeholder
	_dataSet.getCreatePlist().getFillValue(H5::PredType::NATIVE_DOUBLE,
			&_nodata);

	// TODO further checks if required: do dimensions match etc
}

void Hdf5MappingsStore::_createFile() {
	// 1. main data set
	// create data space
	hsize_t dims[4] = { 0, 0, 0, 0 };
	hsize_t maxdims[4] = { H5S_UNLIMITED, H5S_UNLIMITED, H5S_UNLIMITED,
			H5S_UNLIMITED };
	H5::DataSpace dataSpace(4, dims, maxdims);
	// create file
	try {
		_h5file = new H5::H5File(_filename, H5F_ACC_TRUNC);
	} catch (H5::FileIException & e) {
		std::stringstream ss;
		ss << "Failed to create file " << _filename
				<< " (disk full? no write permissions?)";
		throw BadConfigurationException(ss.str());
	}
	// enable chunking & set chunk size
	H5::DSetCreatPropList cparms;
	hsize_t chunks[4] = { 1, 10, 1, 1 };
	cparms.setChunk(4, chunks);
	// set nodata value
	cparms.setFillValue(H5::PredType::NATIVE_DOUBLE, &_nodata);
	// create data set
	_dataSet = _h5file->createDataSet(DATASET_NAME, H5::PredType::NATIVE_DOUBLE,
			dataSpace, cparms);
	// add data set attributes
	std::stringstream ss;
	ss << _firstDateInFile;
	Hdf5Tools::createStringAttribute(_dataSet, START_DATE_NAME,
			ss.str().substr(0, 10));
	Hdf5Tools::createStringAttribute(_dataSet, DIMENSIONS_NAME, DIMENSIONS);
	Hdf5Tools::createStringAttribute(_dataSet, DESCRIPTION_NAME, DESCRIPTION);

	// 2. meta data: parameters & cells
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
	long longNoData = -1;
	cparms2.setFillValue(H5::PredType::NATIVE_LONG, &longNoData);
	_cellsSet = _h5file->createDataSet(CELL_DATASET_NAME, H5::PredType::NATIVE_LONG,
			dataSpace2, cparms2);

	// 3. store cell names into file
	Grid * grid = getGridProvider()->getGrid();
	std::vector<Cell *> * cells = &(grid->getCells());
	std::vector<Cell *>::iterator it = cells->begin();
	while (it != cells->end()) {
		Cell * cell = *it++;
		Hdf5Tools::addToLongDataSet(_cellsSet, cell->getId());
	}
}

void Hdf5MappingsStore::_checkGridVsFile() {
	// get cells from file
	unsigned int bufferSize = Hdf5Tools::getDataSetSize(_cellsSet);
	long * buffer;
	buffer = new long[bufferSize];
	Hdf5Tools::readLongData(buffer, _cellsSet);
	// get cells from grid
	Grid * grid = getGridProvider()->getGrid();
	std::vector<Cell *> * cells = &(grid->getCells());
	// validate size
	if (cells->size() != bufferSize) {
		delete [] buffer;
		throw RunTimeException("Hdf5MappingsStore: grid in file is not equal to the one provided by the grid provider");
	}
	// validate contents
	unsigned int index = 0;
	std::vector<Cell *>::iterator it = cells->begin();
	while (it != cells->end()) {
		Cell * cell = *it++;
		if (cell->getId() != buffer[index++]) {
			delete [] buffer;
			throw RunTimeException("Hdf5MappingsStore: grid in file is not equal to the one provided by the grid provider");
		}
	}
	// clean up buffer
	delete [] buffer;
}

} /* namespace OPAQ */

OPAQ_REGISTER_PLUGIN(OPAQ::Hdf5MappingsStore)

