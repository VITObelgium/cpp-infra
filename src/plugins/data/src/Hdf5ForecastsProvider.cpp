/*
 * Hdf5ForecastsProvider.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "Hdf5ForecastsProvider.h"

namespace OPAQ {

Hdf5ForecastsProvider::Hdf5ForecastsProvider() {
	// tell the hdf5 lib not to print error messages: we will handle them properly ourselves
	H5::Exception::dontPrint();
	_h5file = NULL;
	_aqNetworkProvider = NULL;
	_configured = false;
	_baseTimeSet = false;

}

Hdf5ForecastsProvider::~Hdf5ForecastsProvider() {
	if (_h5file != NULL) {
		_dataSet.close();
		_parametersSet.close();
		_stationsSet.close();
		_h5file->close();
		delete _h5file;
	}
}

const std::string Hdf5ForecastsProvider::DATASET_NAME("forecasted_values");
const std::string Hdf5ForecastsProvider::PARAMETERS_DATASET_NAME("parameters");
const std::string Hdf5ForecastsProvider::STATION_DATASET_NAME("stations");
const std::string Hdf5ForecastsProvider::START_DATE_NAME("start_date");
const std::string Hdf5ForecastsProvider::DIMENSIONS_NAME("dimensions");
const std::string Hdf5ForecastsProvider::DIMENSIONS(
		"parameter x station x base time x forecast horizon");
const std::string Hdf5ForecastsProvider::DESCRIPTION_NAME("description");
const std::string Hdf5ForecastsProvider::DESCRIPTION("OPAQ forecasts");

LOGGER_DEF(Hdf5ForecastsProvider)

// OPAQ::Component methods

void Hdf5ForecastsProvider::configure(TiXmlElement * configuration)
		throw (BadConfigurationException) {

	// 1. parse filename
	TiXmlElement * fileEl = configuration->FirstChildElement("filename");
	if (!fileEl)
		throw BadConfigurationException("filename element not found");
	_filename = fileEl->GetText();

	_configured = true;

//	// 2. open file
//	if (FileTools::exists(filename)) {
//		_openFile(filename);
//	} else {
//		// TODO: allow having no file at this time: open it as soon as we need it: when getting values from it
//		std::stringstream ss;
//		ss << "File not found: " << filename;
//		throw BadConfigurationException(ss.str());
//	}
}

// OPAQ::DataProvider methods

void Hdf5ForecastsProvider::setAQNetworkProvider(
		AQNetworkProvider * aqNetworkProvider) {
	this->_aqNetworkProvider = aqNetworkProvider;
}

void Hdf5ForecastsProvider::setBaseTime(const DateTime & baseTime)
		throw (BadConfigurationException) {
	_baseTime = DateTimeTools::floor(baseTime, DateTimeTools::FIELD_DAY);
	_baseTimeSet = true;
}

TimeInterval Hdf5ForecastsProvider::getTimeResolution() {
	return TimeInterval(86400); // 1 day
}

std::pair<const TimeInterval, const TimeInterval> Hdf5ForecastsProvider::getRange() {
	return getRange(ForecastHorizon(0));
}

std::pair<const TimeInterval, const TimeInterval> Hdf5ForecastsProvider::getRange(
		const ForecastHorizon & forecastHorizon) {
	_checkFullyConfigured();
	DateTime begin = _startDate;
	begin.addSeconds(forecastHorizon.getSeconds());
	DateTime end = begin;
	end.addDays(size() - 1);
	TimeInterval beginOffset(_baseTime, begin);
	TimeInterval endOffset(_baseTime, end);
	return std::pair<const TimeInterval, const TimeInterval>(beginOffset,
			endOffset);
}

unsigned int Hdf5ForecastsProvider::size() {
	_checkFullyConfigured();
	return Hdf5Tools::getDataSetSize(_dataSet, 2);
}

double Hdf5ForecastsProvider::getNoData() {
	_checkFullyConfigured();
	double out;
	_dataSet.getCreatePlist().getFillValue(H5::PredType::NATIVE_DOUBLE, &out);
	return out;
}

double Hdf5ForecastsProvider::getNoData(const std::string & id) {
	return getNoData();
}

std::vector<double> Hdf5ForecastsProvider::getValues(
		const TimeInterval & beginOffset, const TimeInterval & endOffset,
		const std::string & parameter, const std::string & station) {
	return getValues(beginOffset, endOffset, parameter, station,
			ForecastHorizon(0));
}

std::vector<double> Hdf5ForecastsProvider::getValues(
		const TimeInterval & beginOffset, const TimeInterval & endOffset,
		const std::string & parameter, const std::string & station,
		const ForecastHorizon & forecastHorizon) {
	_checkFullyConfigured();
	// 1. validate forecastHorizon
	long days = forecastHorizon.getHours();
	if (days % 24 != 0)
		throw RunTimeException(
				"forecast horizon must be a multiple of 24 hours");
	days /= 24;

	/*
	 * 2. given begin and end dates must be converted into base times using the given forecast horizon
	 * for example: if the forecast horizon equals 5 days, the base time of the requested dates
	 * lies 5 days earlier.
	 * in other words: base time = given day - forecast horizon
	 */
	// a. calculate begin and end from given intervals and the base time
	DateTime begin = _baseTime;
	begin.addSeconds(beginOffset.getSeconds());
	DateTime end = _baseTime;
	end.addSeconds(endOffset.getSeconds());
	// a. first round them to the nearest day boundaries
	DateTime myBegin = DateTimeTools::floor(begin, DateTimeTools::FIELD_DAY);
	DateTime myEnd = DateTimeTools::ceil(end, DateTimeTools::FIELD_DAY);
	// b. then convert them
	myBegin.addDays(-days);
	myEnd.addDays(-days);

	// 3. get parameter, station, date, and forecast horizon indices
	int parameterIndex = Hdf5Tools::getIndexInStringDataSet(_parametersSet,
			parameter);
	int stationIndex = Hdf5Tools::getIndexInStringDataSet(_stationsSet,
			station);
	int btIndex = TimeInterval(_startDate, myBegin).getDays();
	int fhIndex = days;

	// 4. get data
	// a. start with vector filled with nodata
	std::vector<double> out;
	int count = TimeInterval(myBegin, myEnd).getDays() + 1;
	double nodata = getNoData();
	for (int i = 0; i < count; i++) {
		out.push_back(nodata);
	}
	// b. then fill the part for which we have data
	int pSize = Hdf5Tools::getDataSetSize(_dataSet, 0);
	int sSize = Hdf5Tools::getDataSetSize(_dataSet, 1);
	int btSize = Hdf5Tools::getDataSetSize(_dataSet, 2);
	int fhSize = Hdf5Tools::getDataSetSize(_dataSet, 3);

	if (parameterIndex >= 0 && parameterIndex < pSize && stationIndex >= 0
			&& stationIndex < sSize && fhIndex >= 0 && fhIndex < fhSize) {
		int dataStartIndex = btIndex < 0 ? 0 : btIndex;
		int dataEndIndex = btIndex + count - 1;
		if (dataStartIndex < btSize && dataEndIndex >= 0) {
			// only need to fill if the selected hyperslab lies within the data set
			if (dataEndIndex >= btSize)
				dataEndIndex = btSize - 1;
			int dataCount = dataEndIndex - dataStartIndex + 1;
			int outStartIndex = btIndex < 0 ? -btIndex : 0;
			// select hyperslabs and fetch the data
			H5::DataSpace space = _dataSet.getSpace();
			hsize_t dc[4] = { 1, 1, dataCount, 1 };
			hsize_t doffset[4] = { parameterIndex, stationIndex, dataStartIndex,
					fhIndex };
			space.selectHyperslab(H5S_SELECT_SET, dc, doffset);
			hsize_t mc[1] = { count };
			hsize_t moffset[1] = { outStartIndex };
			H5::DataSpace memSpace(1, mc);
			mc[0] = dataCount;
			memSpace.selectHyperslab(H5S_SELECT_SET, mc, moffset);
			_dataSet.read(&out[0], H5::PredType::NATIVE_DOUBLE, memSpace,
					space);
			space.close();
		}
	}

	// 5. return data
	return out;
}

std::vector<double> Hdf5ForecastsProvider::getValues(
		const std::string & parameter, const TimeInterval & offset,
		const ForecastHorizon & forecastHorizon) {
	if (_aqNetworkProvider == NULL) throw RunTimeException("no AQ network provider set, cannot provided values for stations");
	std::vector<double> out;
	Pollutant * pol = Config::PollutantManager::getInstance()->find(parameter);
	AQNetwork * net = _aqNetworkProvider->getAQNetwork();
	std::vector<Station *> * stations = &(net->getStations());
	std::vector<Station *>::iterator it = stations->begin();
	while (it != stations->end()) {
		Station * station = *it++;
		if (pol == NULL || AQNetworkTools::stationHasPollutant(station, *pol)) {
			/*
			 * the parameter is not a pollutant
			 * or
			 * the parameter is a pollutant and it is provided by the station
			 *
			 * in both cases, we need to fetch the data from the file
			 */
			out.push_back(
					getValues(offset, offset, parameter, station->getName(),
							forecastHorizon).front());
		} else {
			/*
			 * the parameter denotes a pollutant, but it is not provided by the station
			 */
			out.push_back(getNoData());
		}
	}
	return out;
	// TODO: can we implement this more efficiently by directly querying the h5 file?
}

void Hdf5ForecastsProvider::_checkFullyConfigured()
		throw (NotConfiguredException) {
	if (!_configured || !_baseTimeSet) throw NotConfiguredException("Not fully configured");
	if (_h5file == NULL) {
		if (FileTools::exists(_filename))
			_openFile(_filename);
		else {
			std::stringstream ss;
			ss << "File not found: " << _filename;
			throw RunTimeException(ss.str());
		}
	}
}

void Hdf5ForecastsProvider::_openFile(const std::string & filename)
		throw (BadConfigurationException) {
	// 1. open file
	try {
		_h5file = new H5::H5File(filename, H5F_ACC_RDONLY);
	} catch (H5::FileIException & e) {
		_h5file = NULL;
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
		_h5file->close();
		_h5file = NULL;
		std::stringstream ss;
		ss << "File " << filename << " does not contain forecasts data";
		throw BadConfigurationException(ss.str());
	}

	// 3. get start date
	try {
		std::string dateStr = Hdf5Tools::readStringAttribute(_dataSet,
				START_DATE_NAME);
		_startDate = DateTimeTools::parseDate(dateStr);
	} catch (H5::AttributeIException & e) {
		_h5file->close();
		_h5file = NULL;
		std::stringstream ss;
		ss << "Failed to get start date from file " << filename
				<< " (file corrupted?)";
		throw BadConfigurationException(ss.str());
	} catch (ParseException & e) {
		_h5file->close();
		_h5file = NULL;
		std::stringstream ss;
		ss << "Failed to parse date in start_date attribute in file "
				<< filename << " (file corrupted?)";
		throw BadConfigurationException(ss.str());
	}

	// TODO: more checks?
}

} /* namespace OPAQ */

OPAQ_REGISTER_PLUGIN(OPAQ::Hdf5ForecastsProvider);

