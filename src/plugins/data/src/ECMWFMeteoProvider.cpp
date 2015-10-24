/*
 * ECMWFMeteoProvider.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "ECMWFMeteoProvider.h"

namespace OPAQ {

const std::string ECMWFMeteoProvider::METEO_PLACEHOLDER = "%METEO%";
const std::string ECMWFMeteoProvider::PARAMETER_PLACEHOLDER = "%PARAMETER%";

LOGGER_DEF(ECMWFMeteoProvider)

ECMWFMeteoProvider::ECMWFMeteoProvider() {
	_timeResolution = TimeInterval(3600 * 6); // ECMWF meteo data has 6-hourly resolution
	_defaultNodata = -9999;
	_configured = false;
	_baseTimeSet = false;
}

ECMWFMeteoProvider::~ECMWFMeteoProvider() {
}

// OPAQ::Component methods

void ECMWFMeteoProvider::configure(TiXmlElement * configuration)
		throw (BadConfigurationException) {
	// 1. parse range configuration

	TiXmlElement * rangeElement = configuration->FirstChildElement("range");
	if (!rangeElement)
		throw BadConfigurationException("range element not found");

	TiXmlElement * beginOffsetElement = rangeElement->FirstChildElement("begin_offset");
	if (!beginOffsetElement)
		throw BadConfigurationException("begin_offset element not found");
	_beginOffset = atoi(beginOffsetElement->GetText());

	TiXmlElement * endOffsetElement = rangeElement->FirstChildElement("end_offset");
	if (!endOffsetElement)
		throw BadConfigurationException("end_offset element not found");
	_endOffset = atoi(endOffsetElement->GetText());

	if (_beginOffset > _endOffset)
		throw BadConfigurationException("Invalid before-after configuration");

	if (_baseTimeSet)
		_calculateBeginEnd();

	// 2. parse parameter configuration
	_nodata.clear();
	TiXmlElement * parametersElement = configuration->FirstChildElement(
			"parameters");
	if (!parametersElement)
		throw BadConfigurationException("parameters element not found");
	TiXmlElement * pEl = parametersElement->FirstChildElement("parameter");
	while (pEl) {
		std::string id;
		double nodata;

		if (pEl->QueryStringAttribute("id", &id) != TIXML_SUCCESS
				|| pEl->QueryDoubleAttribute("nodata", &nodata)
						!= TIXML_SUCCESS)
			throw BadConfigurationException(
					"parameter must have id and nodata attributes defined");

		_nodata.insert(std::pair<std::string, double>(id, nodata));

		pEl = pEl->NextSiblingElement("parameter");
	}

	// 3. parse data file pattern
	TiXmlElement * patternElement = configuration->FirstChildElement(
			"file_pattern");
	if (!patternElement)
		throw BadConfigurationException("file_pattern element not found");
	_pattern = patternElement->GetText();

	// 4. clear buffers
	_buffer.clear();
	_emptyBuffer.clear();

	_configured = true;

}

// OPAQ::DataProvider methods

void ECMWFMeteoProvider::setAQNetworkProvider(
		AQNetworkProvider * aqNetworkProvider) {
	// Nothing to do here, meteo doesn't need an AQNetwork
}

void ECMWFMeteoProvider::setBaseTime(const DateTime & baseTime)
		throw (BadConfigurationException) {
	_baseTime = baseTime;
	_floor6(_baseTime);
	if (_configured)
		_calculateBeginEnd();
	_baseTimeSet = true;
	// new base time set -> clear the buffer
	_buffer.clear();
}

TimeInterval ECMWFMeteoProvider::getTimeResolution() {
	return _timeResolution;
}

std::pair<const TimeInterval, const TimeInterval> ECMWFMeteoProvider::getRange() {
	_checkFullyConfigured();
	return std::pair<const TimeInterval, const TimeInterval>(
			TimeInterval(_beginOffset * 6 * 60 * 60),
			TimeInterval(_endOffset * 6 * 60 * 60));
}

std::pair<const TimeInterval, const TimeInterval> ECMWFMeteoProvider::getRange(
		const ForecastHorizon & forecastHorizon) {
	return getRange();
}

unsigned int ECMWFMeteoProvider::size() {
	_checkFullyConfigured();
	TimeInterval ti(_begin, _end);
	int size = ti.getHours() / 6 + 1;
	return size < 0 ? 0 : size;
	/*
	 * for example: 6h00 - 0h00: ti.getHours() = 18 hours
	 * /6 cuz we have 6-hour resolution
	 * + 1 cuz we include both boundaries
	 */
}

double ECMWFMeteoProvider::getNoData() {
	throw NotAvailableException("Use getNoData(const std::string &) instead");
}

double ECMWFMeteoProvider::getNoData(const std::string & parameterId) {
	_checkFullyConfigured();
	std::map<std::string, double>::iterator it = _nodata.find(parameterId);
	if (it == _nodata.end())
		return _defaultNodata;
	else
		return it->second;
}

std::vector<double> ECMWFMeteoProvider::getValues(const TimeInterval & beginOffset,
		const TimeInterval & endOffset, const std::string & meteoId,
		const std::string & parameterId) {
	_checkFullyConfigured();
	if (beginOffset.getSeconds() > endOffset.getSeconds())
		throw RunTimeException ("begin offset must be <= end offset");
	// calculate begin and end from given time intervals and base time
	DateTime begin = _baseTime; begin.addSeconds(beginOffset.getSeconds());
	DateTime end = _baseTime; end.addSeconds(endOffset.getSeconds());
	// round boundaries to the nearest "6"-hour boundaries, therby including the
	// given boundaries in the interval
	DateTime myBegin = begin;
	_floor6(myBegin);
	DateTime myEnd = end;
	_ceil6(myEnd);
	// get buffered data
	std::vector<double> data = _getValues(meteoId, parameterId);
	// get starting index in buffer
	TimeInterval it(_begin, myBegin);
	int index = it.getHours() / 6;
	std::vector<double>::iterator bufferIt = data.begin();
	if (index > 0) {
		for (int i = 0; i < index && bufferIt != data.end(); i++)
			bufferIt++;
	}
	/*
	 * select time range within buffer or add values if the
	 * requested begin and/or end lie outside the buffered range
	 */
	std::vector<double> out;
	double nodata = getNoData(parameterId);
	// 1. add nodata before buffered range (if required)
	DateTime pointer = myBegin;
	while (pointer < _begin && pointer <= myEnd) {
		out.push_back(nodata);
		pointer.addHours(6);
	}
	// 2. copy available data from buffer (if required)
	while (pointer <= myEnd && bufferIt != data.end()) {
		out.push_back(*bufferIt);
		pointer.addHours(6);
		bufferIt++;
	}
	// 3. add nodata after buffered range (if required)
	while (pointer <= myEnd) {
		out.push_back(nodata);
		pointer.addHours(6);
	}
	return out;
}

std::vector<double> ECMWFMeteoProvider::getValues(const TimeInterval & beginOffset,
		const TimeInterval & endOffset, const std::string & meteoId,
		const std::string & parameterId,
		const ForecastHorizon & forecastHorizon) {
	return getValues(beginOffset, endOffset, meteoId, parameterId);
}

void ECMWFMeteoProvider::_checkFullyConfigured()
		throw (NotConfiguredException) {
	if (!_configured || !_baseTimeSet)
		throw NotConfiguredException("Not fully configured");
}

void ECMWFMeteoProvider::_calculateBeginEnd() {
	_begin = _baseTime;
	_begin.addHours(_beginOffset * 6);
	_end = _baseTime;
	_end.addHours(_endOffset * 6);
}

void ECMWFMeteoProvider::_floor6(DateTime & datetime) {
	datetime = DateTimeTools::floor(datetime, DateTimeTools::FIELD_HOUR);
	if (datetime.getHour() >= 18)
		datetime.setHour(18);
	else if (datetime.getHour() >= 12)
		datetime.setHour(12);
	else if (datetime.getHour() >= 6)
		datetime.setHour(6);
	else
		datetime.setHour(0);
}

void ECMWFMeteoProvider::_ceil6(DateTime & datetime) {
	datetime = DateTimeTools::ceil(datetime, DateTimeTools::FIELD_HOUR);
	if (datetime.getHour() > 18)
		datetime.addHours(24 - datetime.getHour());
	else if (datetime.getHour() > 12)
		datetime.setHour(18);
	else if (datetime.getHour() > 6)
		datetime.setHour(12);
	else if (datetime.getHour() > 0)
		datetime.setHour(6);
}

const std::vector<double> & ECMWFMeteoProvider::_getValues(
		const std::string & meteoId, const std::string & parameterId) {
	std::map<std::string, std::map<std::string, std::vector<double> > >::iterator meteoIt =
			_buffer.find(meteoId);
	if (meteoIt == _buffer.end()) {
		// not found: read from data file
		_readFile(meteoId, parameterId);
		// and fetch it again
		meteoIt = _buffer.find(meteoId);
	}
	if (meteoIt == _buffer.end()) {
		// no data available for this meteoId: return empty vector
		return _getEmpty(parameterId);
	}
	std::map<std::string, std::vector<double> > * buffer = &(meteoIt->second);
	std::map<std::string, std::vector<double> >::iterator parameterIt =
			buffer->find(parameterId);
	if (parameterIt == buffer->end()) {
		// not found: read from data file
		_readFile(meteoId, parameterId);
		// and fetch it again
		parameterIt = buffer->find(parameterId);
	}
	if (parameterIt == buffer->end()) {
		// no data available for this meteoId/parameterId combination: return empty vector
		return _getEmpty(parameterId);
	} else {
		return parameterIt->second;
	}
}

void ECMWFMeteoProvider::_readFile(const std::string & meteoId,
		const std::string & parameterId) {
	// create file name
	std::string filename = _pattern;
	StringTools::replaceAll(filename, METEO_PLACEHOLDER, meteoId);
	StringTools::replaceAll(filename, PARAMETER_PLACEHOLDER, parameterId);
	// read & parse file
	GzipReader reader;
	try {
		reader.open(filename);
	} catch (IOException & e) {
		std::stringstream ss;
		ss << "File not found: " << filename;
		logger->warn(ss.str());
	}
	std::string line = reader.readLine();
	while (!reader.eof()) {
		/*
		 * line format:
		 * YYYYMMDD hour0 hour6 hour12 hour18
		 */
		std::vector<std::string> tokens = StringTools::tokenize(line);
		if (tokens.size() == 5) { // only parse valid lines
			// only parse lines within the time range of the buffer
			DateTime begin(atoi(tokens[0].substr(0, 4).c_str()),
					atoi(tokens[0].substr(4, 2).c_str()),
					atoi(tokens[0].substr(6, 2).c_str()));
			DateTime end = begin;
			end.addHours(18);
			if (begin <= _end && end >= _begin) {
				// get target vector from buffer
				std::vector<double> * values = _getOrInitValues(meteoId,
						parameterId);
				// determine indices
				int inputBeginIndex = 0;
				while (begin < _begin) {
					begin.addHours(6);
					inputBeginIndex++;
				}
				int inputEndIndex = 3;
				while (end > _end) {
					end.addHours(-6);
					inputEndIndex--;
				}
				int outputBeginIndex = 0;
				DateTime dt = _begin;
				while (dt < begin) {
					dt.addHours(6);
					outputBeginIndex++;
				}
				// copy data
				for (int i = 0; i < (inputEndIndex - inputBeginIndex + 1);
						i++) {
					(*values)[outputBeginIndex + i] = atof(
							tokens[1 + inputBeginIndex + i].c_str());
				}
			}
		}
		line = reader.readLine();
	}
}

std::vector<double> * ECMWFMeteoProvider::_getOrInitValues(
		const std::string & meteoId, const std::string &parameterId) {
	// fetch the data array for the given meteoId and parameterId
	// if not found: create 1 containing all missing values
	std::map<std::string, std::vector<double> > * meteoData;
	std::map<std::string, std::map<std::string, std::vector<double> > >::iterator meteoIt =
			_buffer.find(meteoId);
	if (meteoIt == _buffer.end()) {
		meteoData =
				&(_buffer.insert(
						std::pair<std::string,
								std::map<std::string, std::vector<double> > >(
								meteoId,
								std::map<std::string, std::vector<double> >())).first->second);
	} else {
		meteoData = &(meteoIt->second);
	}
	// get (or create) vector to store the values
	std::vector<double> * out;
	std::map<std::string, std::vector<double> >::iterator parameterIt =
			meteoData->find(parameterId);
	if (parameterIt == meteoData->end()) {
		// not found: create new one with all nodata
		out = &(meteoData->insert(
				std::pair<std::string, std::vector<double> >(parameterId,
						std::vector<double>())).first->second);
		double nodata = getNoData(parameterId);
		for (unsigned int i = 0; i < size(); i++)
			out->push_back(nodata);
	} else {
		out = &(parameterIt->second);
	}
	return out;
}

std::vector<double> & ECMWFMeteoProvider::_getEmpty(
		const std::string & parameterId) {
	// must be buffered cuz the _getValues method returns data by reference
	// if we don't buffer, this will result in dangling pointers
	std::vector<double> * empty;
	std::map<std::string, std::vector<double> >::iterator it =
			_emptyBuffer.find(parameterId);
	if (it == _emptyBuffer.end()) {
		// not found: create new one
		std::map<std::string, double>::iterator it2 = _nodata.find(parameterId);
		double nodata = it2 == _nodata.end() ? _defaultNodata : it2->second;
		empty = &(_emptyBuffer.insert(
				std::pair<std::string, std::vector<double> >(parameterId,
						std::vector<double>())).first->second);
		for (unsigned int i = 0; i < size(); i++)
			empty->push_back(nodata);
	} else {
		empty = &(it->second);
	}
	return *empty;
}

} /* namespace OPAQ */

OPAQ_REGISTER_PLUGIN(OPAQ::ECMWFMeteoProvider);

