/*
 * AsciiObsProvider.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "AsciiObsProvider.h"

namespace OPAQ {

  AsciiObsProvider::AsciiObsProvider() {
    _noData = -9999; // ASCII observations use -9999 as nodata placeholder
    _timeResolution = TimeInterval(3600); // default hourly time interval for observations
    _configured = false;
    _baseTimeSet = false;
    _clearBufferOnBaseTimeReset = true;
    _startCol = 2; // default starting column for hourly values
    _scaleValue = 1; // scale value to convert to µg/m3
  }

  AsciiObsProvider::~AsciiObsProvider() {
  }
  
  LOGGER_DEF(AsciiObsProvider);
  
  const std::string AsciiObsProvider::POLLUTANT_PLACEHOLDER = "%POL%";
  const std::string AsciiObsProvider::STATION_PLACEHOLDER   = "%STATION%";

  // OPAQ::Component methods

  void AsciiObsProvider::configure(TiXmlElement * configuration)
    throw (BadConfigurationException) {

    logger->trace( "Configuring ascii observation provider..." );

    // ---------------------------------------------------------------------
    // 1. parse range configuration
    // ---------------------------------------------------------------------
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
      throw BadConfigurationException("begin offset must be <= end offset");
    
    if (_baseTimeSet)
      _calculateBeginEnd();
    
    // ---------------------------------------------------------------------
    // 2. parse data file pattern
    // ---------------------------------------------------------------------
    TiXmlElement * patternElement = configuration->FirstChildElement("file_pattern");
    if (!patternElement)
      throw BadConfigurationException("file_pattern element not found");
    _pattern = patternElement->GetText();
    

    // ---------------------------------------------------------------------
    // 3. parse optional clear buffer upon base time reset field
    // ---------------------------------------------------------------------
    TiXmlElement * clearElement = configuration->FirstChildElement("clear_on_new_basetime");
    if (clearElement) {
      std::string value = patternElement->GetText();
      std::transform(value.begin(), value.end(), value.begin(), ::toupper);
      _clearBufferOnBaseTimeReset = value.compare("TRUE") == 0;
    }
    

    // ---------------------------------------------------------------------
    // 4. parse starting column for hourly data
    // ---------------------------------------------------------------------
    TiXmlElement * startColElement = configuration->FirstChildElement("start_column");
    if (startColElement) {
      _startCol = atoi(startColElement->GetText());
      if ( _startCol < 1 ) throw BadConfigurationException("starting column should start counting from 1");
    } else {
      logger->warn( "No start_column given in config, using default value : " + std::to_string(_startCol) );
    }


    // ---------------------------------------------------------------------
    // 5. setting time resollution from configuration file ?
    //    value given in configuration is in minutes !
    // ---------------------------------------------------------------------
    TiXmlElement * timeResElement = configuration->FirstChildElement("time_interval");
    if (timeResElement) {
      int interval = atoi(timeResElement->GetText());
      if ( interval != 60 ) {

	//TODO: NOT IMPLEMENTED YET...
	// need to carefully check the following code whether it supports resolutions other than 60 minutes...
	throw BadConfigurationException( "Time resolution other than 60 minutes is not fully implemented yet" );

      }

      _timeResolution = TimeInterval( 60 * interval );
    } else {
      logger->warn( "No time resolution given in config, using default value (min) : " + 
		    std::to_string( _timeResolution.getMinutes() ) );
    }


    // ---------------------------------------------------------------------
    // 4. parse scale value
    // ---------------------------------------------------------------------
    TiXmlElement * scaleElement = configuration->FirstChildElement("scale");
    if (scaleElement) {
      _scaleValue = atof(scaleElement->GetText());
    } else {
      logger->warn( "No scale value given in config, using default value : " + std::to_string(_scaleValue) );
    }


    // 4. clear buffer
    _buffer.clear();
    
    _configured = true;
  }

  // OPAQ::DataProvider methods

  void AsciiObsProvider::setAQNetworkProvider(AQNetworkProvider * aqNetworkProvider) {
    this->_aqNetworkProvider = aqNetworkProvider;
  }

  void AsciiObsProvider::setBaseTime(const DateTime & baseTime)
    throw (BadConfigurationException) {
    _baseTime = DateTimeTools::floor(baseTime, DateTimeTools::FIELD_HOUR);
    // ignore this if buffer doesn't need to be cleared and the base time is already set
    if (_clearBufferOnBaseTimeReset || !_baseTimeSet) {
      if (_configured)
	_calculateBeginEnd();
      _baseTimeSet = true;
      // new base time set -> clear the buffer
      _buffer.clear();
    }
  }

  TimeInterval AsciiObsProvider::getTimeResolution() {
    return _timeResolution;
  }
  
  std::pair<const TimeInterval, const TimeInterval> AsciiObsProvider::getRange() {
    _checkFullyConfigured();
    return std::pair<const TimeInterval, 
		     const TimeInterval>( TimeInterval(_beginOffset * 3600), 
					  TimeInterval(_endOffset * 3600));
  }

  std::pair<const TimeInterval, 
	    const TimeInterval> AsciiObsProvider::getRange(const ForecastHorizon & forecastHorizon) {
    return getRange();
  }

  unsigned int AsciiObsProvider::size() {
    _checkFullyConfigured();
    TimeInterval ti(_begin, _end);
    return ti.getHours() + 1; // time interval from 20h00 to 21h00 lasts 1 hour but contains 2 hours
  }

  double AsciiObsProvider::getNoData() {
    return _noData;
  }
  
  double AsciiObsProvider::getNoData(const std::string & id) {
    return getNoData();
  }
  
  std::vector<double> AsciiObsProvider::getValues(const TimeInterval & beginOffset,
						  const TimeInterval & endOffset, 
						  const std::string & pollutant,
						  const std::string & station) {
    _checkFullyConfigured();
    if (beginOffset.getSeconds() > endOffset.getSeconds())
      throw RunTimeException("beginOffset must be <= endOffset");
    // calculate begin and end from given offsets
    DateTime begin = _baseTime;
    begin.addSeconds(beginOffset.getSeconds());
    DateTime end = _baseTime;
    end.addSeconds(endOffset.getSeconds());
    // round boundaries to the nearest hours, thereby including the
    // given boundaries in the interval
    DateTime myBegin = DateTimeTools::floor(begin, DateTimeTools::FIELD_HOUR);
    DateTime myEnd = DateTimeTools::ceil(end, DateTimeTools::FIELD_HOUR);
    // get buffered data
    std::vector<double> * data = &_getValues(pollutant, station);
    // get starting index in buffer
    TimeInterval it(_begin, myBegin);
    int index = it.getHours();
    std::vector<double>::iterator bufferIt = data->begin();
    if (index > 0) {
      for (int i = 0; i < index && bufferIt != data->end(); i++)
	bufferIt++;
    }
    /*
     * select time range within buffer or add values
     * if requested begin or end lie outside the buffered range
     */
    std::vector<double> out;
    // 1. add noData before buffered range (if required)
    DateTime pointer = myBegin;
    while (pointer < _begin && pointer <= myEnd) {
      out.push_back(_noData);
      pointer.addHours(1);
    }
    // 2. copy available data from buffer (if required)
    while (pointer <= myEnd && bufferIt != data->end()) {
      out.push_back(*bufferIt);
      pointer.addHours(1);
      bufferIt++;
    }
    // 3. add noData after buffered range (if required)
    while (pointer <= myEnd) {
      out.push_back(_noData);
      pointer.addHours(1);
    }
    return out;
  }
  
  std::vector<double> AsciiObsProvider::getValues(const TimeInterval & beginOffset,
						  const TimeInterval & endOffset, 
						  const std::string & pollutant,
						  const std::string & station, 
						  const ForecastHorizon & forecastHorizon) {
    return getValues(beginOffset, endOffset, pollutant, station);
  }

std::vector<double> AsciiObsProvider::getValues(const std::string & pollutant,
						const TimeInterval & offset, 
						const ForecastHorizon & forecastHorizon) {
  std::vector<double> out;
  AQNetwork * net = _aqNetworkProvider->getAQNetwork();
  Pollutant * pol = Config::PollutantManager::getInstance()->find(pollutant);
  std::vector<Station *> * stations = &(net->getStations());
  std::vector<Station *>::iterator it = stations->begin();
  while (it != stations->end()) {
    Station * station = *it++;
    if (AQNetworkTools::stationHasPollutant(station, *pol)) {
      // station has data for the given pollutant
      out.push_back(getValues(offset, offset, pollutant, station->getName(),forecastHorizon).front());
    } else {
      // station doesn't have data for the given pollutant
      out.push_back(_noData);
    }
  }
  return out;
}

void AsciiObsProvider::_checkFullyConfigured() throw (NotConfiguredException) {
  if (!_configured || !_baseTimeSet)
    throw NotConfiguredException("Not fully configured");
}
  
void AsciiObsProvider::_calculateBeginEnd() {
  _begin = _baseTime;
  _begin.addHours(_beginOffset);
  _end = _baseTime;
  _end.addHours(_endOffset);
  
  // now that we have _begin and _end, we can initialize _empty as well
  // set empty data vector (to return when stations/pollutants/time ranges
  // are requested that are not available.
  TimeInterval ti(_begin, _end);
  unsigned int hourCount = ti.getHours() + 1; // 20:00:00 - 21:00:00 is a 1 hour interval but it has two hours of data
  if (_empty.size() > hourCount) {
    while (_empty.size() > hourCount)
      _empty.pop_back();
  } else {
    while (_empty.size() < hourCount)
      _empty.push_back(_noData);
  }
}
  
  std::vector<double> & AsciiObsProvider::_getValues(const std::string & pollutant,
						     const std::string & station) {
    std::map<std::string, std::map<std::string, std::vector<double> > >::iterator it =
      _buffer.find(pollutant);

    if (it == _buffer.end()) {
      // not found: read data file
      _readFile(pollutant, station);
      // and fetch it again
      it = _buffer.find(pollutant);
    }
    if (it == _buffer.end()) {
      // no data available for this pollutant: return empty vector
      return _empty;
    }
    std::map<std::string, std::vector<double> > * buffer = &(it->second);
    std::map<std::string, std::vector<double> >::iterator it2 = buffer->find(station);
    if (it2 == buffer->end()) {
      // read from file
      _readFile(pollutant,station);
      // and fetch it again
      it2 = buffer->find(station);
    }
    if ( it2 == buffer->end() ) {
      // no data available for this station/pollutant combination: return empty vector
      return _empty;
    } else {
      return it2->second;
    }
  }
  
  void AsciiObsProvider::_readFile(const std::string & pollutant, const std::string & station ) {
  // create file name
  std::string filename = _pattern;
  StringTools::replaceAll(filename, POLLUTANT_PLACEHOLDER, pollutant);
  StringTools::replaceAll(filename, STATION_PLACEHOLDER, station );
  std::ifstream file(filename.c_str());
  // read & parse file
  if (file.is_open()) {

    std::string line;
    while (getline(file, line)) {


      /*
       * line format:
       * 1              <start_col> ....
       * 0123456789
       * YYYY-MM-DD ? ? ? hour0 hour1, ..., hour23
       * the starting column can be configured in the configuration file via
       * <start_column>
       * the value indicates the column where the hourly values start
       */
      std::vector<std::string> tokens = StringTools::tokenize(line);

      // only parse lines within the time interval of the buffer      
      DateTime begin(atoi(tokens[0].substr(0, 4).c_str()),
		     atoi(tokens[0].substr(5, 2).c_str()),
		     atoi(tokens[0].substr(8, 2).c_str()), 0, 0, 0);
      DateTime end = begin;
      end.addHours(23);

      if (begin <= _end && end >= _begin) {

	// get target vector from buffer
	std::vector<double> * values = _getOrInitValues(pollutant,station);
	  // determine indices
	  int inputBeginIndex = 0;
	  while (begin < _begin) {
	    begin.addHours(1);
	    inputBeginIndex++;
	  }
	  int inputEndIndex = 23;
	  while (end > _end) {
	    end.addHours(-1);
	    inputEndIndex--;
	  }
	  int outputBeginIndex = 0;
	  DateTime dt = _begin;
	  while (dt < begin) {
	    dt.addHours(1);
	    outputBeginIndex++;
	  }
	  // copy data, apply scale value
	  for (int i = 0; i < (inputEndIndex - inputBeginIndex + 1); i++) {
	    (*values)[outputBeginIndex + i] = atof(tokens[(_startCol - 1) + inputBeginIndex + i].c_str());
	    if ( (*values)[outputBeginIndex + i] != _noData ) (*values)[outputBeginIndex + i] *= _scaleValue;
	  }
	}
      
    }
  } else {
    std::stringstream ss;
    ss << "Failed to open file " << filename;
    logger->warn(ss.str());
  }
}
  
  std::vector<double> * AsciiObsProvider::_getOrInitValues(const std::string & pollutant, 
							   const std::string & station) {
    // get (or create) data array to store the pollutant data
    std::map<std::string, std::vector<double> > * pollutantData;
    std::map<std::string, std::map<std::string, std::vector<double> > >::iterator it =
      _buffer.find(pollutant);

    if (it == _buffer.end()) {
      pollutantData = 
	&(_buffer.insert( std::pair<std::string, std::map<std::string, std::vector<double> > > 
			  ( pollutant, std::map<std::string, std::vector<double> >())).first->second);
    } else {
      pollutantData = &(it->second);
    }

    // get (or create) vector to store the values
    std::vector<double> * out;
    std::map<std::string, std::vector<double> >::iterator it2 = pollutantData->find(station);
    if (it2 == pollutantData->end()) {
      // vector not found: init it with all missing data
      out = &(pollutantData->insert( std::pair<std::string, std::vector<double> >
				     (station, std::vector<double>())).first->second);
      for (unsigned int i = 0; i < size(); i++) {
	out->push_back(_noData);
      }
    } else {
      out = &(it2->second);
    }
    return out;
  }
  
} /* namespace OPAQ */

OPAQ_REGISTER_PLUGIN(OPAQ::AsciiObsProvider);
