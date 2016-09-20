/*
 * IRCELMeteoProvider.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "IRCELMeteoProvider.h"

namespace OPAQ {

const std::string IRCELMeteoProvider::METEO_PLACEHOLDER     = "%meteo%";
const std::string IRCELMeteoProvider::PARAMETER_PLACEHOLDER = "%param%";
const std::string IRCELMeteoProvider::BASETIME_PLACEHOLDER  = "%basetime%";

LOGGER_DEF(IRCELMeteoProvider)

IRCELMeteoProvider::IRCELMeteoProvider() :
	_configured(false),
	_bufferStartReq(false),
	_nsteps(0),
	_backsearch(3) {
}

IRCELMeteoProvider::~IRCELMeteoProvider() {
}

// OPAQ::Component methods
void IRCELMeteoProvider::configure(TiXmlElement * configuration) {

	// clear buffers
	_nodata.clear();
	_buffer.clear();

	// parse parameter configuration
	TiXmlElement * parametersElement = configuration->FirstChildElement( "parameters" );
	if (!parametersElement)	throw BadConfigurationException("parameters element not found");
	TiXmlElement * pEl = parametersElement->FirstChildElement("parameter");
	while (pEl) {
		std::string id;
		double nodata;

		if ( pEl->QueryStringAttribute("id", &id) != TIXML_SUCCESS ||
			 pEl->QueryDoubleAttribute("nodata", &nodata) != TIXML_SUCCESS )
			throw BadConfigurationException( "parameter must have id and nodata attributes defined" );

		_nodata.insert(std::pair<std::string, double>(id, nodata));

		pEl = pEl->NextSiblingElement("parameter");
	}

	// -- parse data file pattern
	TiXmlElement * patternElement = configuration->FirstChildElement( "file_pattern" );
	if (!patternElement) throw BadConfigurationException( "file_pattern element not found" );
	_pattern = patternElement->GetText();

	// -- parsed the backward_search
	TiXmlElement *backSearchEl = configuration->FirstChildElement( "backward_search" );
	if ( backSearchEl ) _backsearch = atoi( backSearchEl->GetText() );


	// -- parse meteo time resolution, value in hours
	TiXmlElement *resEl = configuration->FirstChildElement( "resolution" );
	if ( ! resEl ) throw BadConfigurationException( "resolution not given" );
	int res = atoi( resEl->GetText() );
	if ( ( res != 1 ) && ( res != 2 ) && ( res != 3 ) && ( res != 4 ) && ( res != 6 ) && ( res != 8 ) )
		throw BadConfigurationException( "invalid resolution, valid values are 1, 2, 3, 4, 6 and 8 hours !" );
	_timeResolution = OPAQ::TimeInterval( res, TimeInterval::Hours );
	_nsteps = 24/res;

	// -- parse start time
	TiXmlElement *dateEl = configuration->FirstChildElement( "buffer_start" );
	if ( dateEl ) {
		_bufferStartDate = OPAQ::DateTime( dateEl->GetText() );
		_bufferStartReq  = true;
	}

	// -- set the configured flag to true
	_configured = true;
}

const OPAQ::TimeInterval& IRCELMeteoProvider::getTimeResolution() {
	return _timeResolution;
}


double IRCELMeteoProvider::getNoData( const std::string & parameterId ) {
	_checkConfig();
	auto it = _nodata.find(parameterId);
	if ( it == _nodata.end() )
		OPAQ::NotAvailableException( "Meteo parameter " + parameterId + "is not configured !" );
	return it->second;
}

OPAQ::TimeSeries<double> IRCELMeteoProvider::getValues(	const DateTime& t1,
														const DateTime& t2,
														const std::string& meteoId,
														const std::string& paramId ) {

	if ( t2 < t1 ) throw RunTimeException( "end date has to be after start date" );

	_checkConfig();

	// do we have the data in the buffer for this meteo ID ?
	auto meteoIt = _buffer.find( meteoId );
	if ( meteoIt == _buffer.end() ) {
		// this partiular meteo id was not found in the buffer, read it from the file
		_readFile( meteoId, paramId );
		// and fetch it again
		meteoIt = _buffer.find(meteoId);
	}

	// no data available for this meteoId: return empty vector
	if ( meteoIt == _buffer.end() ) {
		OPAQ::TimeSeries<double> empty;	empty.clear();
		return empty;
	}

	// get buffered data
	OPAQ::TimeSeries<double> ts = _getTimeSeries( meteoId, paramId );

	// return the requested part
	return ts.select( t1, t2 );
}

void IRCELMeteoProvider::_checkConfig() {
	if (!_configured )
		throw NotConfiguredException("Not fully configured");
}


const OPAQ::TimeSeries<double> & IRCELMeteoProvider::_getTimeSeries( const std::string & meteoId,
		  	  	  	  	  	  	  	  	  	  	  	  	  		     const std::string & parameterId ) {

	auto meteoIt = _buffer.find(meteoId);
	if (meteoIt == _buffer.end()) {
		// not found: read from data file
		_readFile(meteoId, parameterId);
		// and fetch it again
		meteoIt = _buffer.find(meteoId);
	}
	if (meteoIt == _buffer.end()) {
		// no data available for this meteoId: return empty vector
		throw NotAvailableException("Requested meteoId not available in ECMWF data set...");
	}

	std::map<std::string, OPAQ::TimeSeries<double> > *buffer = &(meteoIt->second);
	auto parameterIt = buffer->find(parameterId);
	if (parameterIt == buffer->end()) {
		// not found: read from data file
		_readFile(meteoId, parameterId);
		// and fetch it again
		parameterIt = buffer->find(parameterId);
	}
	if (parameterIt == buffer->end()) {
		// no data available for this meteoId/parameterId combination: return empty vector
		throw NotAvailableException( "Requested parameter not available in ECMWF data set..." );
	} else {
		return parameterIt->second;
	}
}

void IRCELMeteoProvider::_readFile( const std::string & meteoId,
									const std::string & parameterId ) {

	GzipReader reader;
	std::string filename;

	// -- Create file name for the current basetime, if it doesn't exist, check the previous
	//    basetime, up till the amount of days configured...
	OPAQ::DateTime checkDate = _baseTime;
	bool have_file = false;
	while (  ( checkDate >= _baseTime-OPAQ::TimeInterval( _backsearch, OPAQ::TimeInterval::Days ) )
			&& ( ! have_file ) ) {

		filename = _pattern;
		StringTools::replaceAll(filename, METEO_PLACEHOLDER, meteoId);
		StringTools::replaceAll(filename, PARAMETER_PLACEHOLDER, parameterId);
		StringTools::replaceAll(filename, BASETIME_PLACEHOLDER, checkDate.dateToString() );

		// -- Read & parse file
		try {
			reader.open(filename);
			have_file = true;
		} catch (const IOException &) {
			checkDate = checkDate - OPAQ::TimeInterval( 1, OPAQ::TimeInterval::Days );
			logger->warn( filename + " not found, checking previous day : " + checkDate.dateToString() );
		}
	}

	if ( ! have_file ) {
		logger->error( "giving up : no meteo file found for " + meteoId + ", " + parameterId );
		return;
	}

	// -- Get the timeseries for this meteo/parameter buffer
	OPAQ::TimeSeries<double> *ts = _getOrInit( meteoId, parameterId );

	std::string line = reader.readLine();
	while ( ! reader.eof() ) {
		/*
		 * line format:
		 * YYYYMMDD hour0 hour6 hour12 hour18     // older ECMWF data : every 6 hours
		 * YYYYMMDD hour0 hour3 hour6 hour9 ...   // new ECMWF data : every 3 hours
		 */
		std::vector<std::string> tokens = OPAQ::StringTools::tokenize( line );
		int nh = static_cast<int>(tokens.size()) - 1;

		if ( nh == _nsteps ) {
			// parse the first token : the date
			DateTime begin( atoi( tokens[0].substr(0, 4).c_str() ),
					        atoi( tokens[0].substr(4, 2).c_str() ),
						    atoi( tokens[0].substr(6, 2).c_str() ) );

			// check if we are beyond the buffer start (if requested...)
			if ( _bufferStartReq && ( begin < _bufferStartDate ) ) {
				line = reader.readLine();
				continue;
			}

			// read in the line & insert into the database
			for ( unsigned int i = 1; i < tokens.size(); i++ ) {
				if ( i > 1 ) begin.addHours( 24/_nsteps );

				// insert into the buffer
				ts->insert( begin, atof( tokens[i].c_str() ) );
			}

		} else {
			throw RunTimeException("format does not match the configuration");
		}

		// read next line...
		line = reader.readLine();
	}
}

OPAQ::TimeSeries<double>* IRCELMeteoProvider::_getOrInit( const std::string & meteoId, const std::string &parameterId ) {

	// fetch the data array for the given meteoId and parameterId
	// if not found: create 1 containing all missing values
	std::map<std::string, OPAQ::TimeSeries<double> > * meteoData;
	auto meteoIt = _buffer.find(meteoId);
	if (meteoIt == _buffer.end()) {
		meteoData =	&(_buffer.insert( std::pair<std::string, std::map<std::string, OPAQ::TimeSeries<double> > >(
								meteoId,
								std::map<std::string, OPAQ::TimeSeries<double> >())).first->second);
	} else {
		meteoData = &(meteoIt->second);
	}

	// get (or create) vector to store the values, do not initialise, this will be done with the insert...
	OPAQ::TimeSeries<double> * out;
	auto parameterIt = meteoData->find(parameterId);
	if (parameterIt == meteoData->end()) {
		out = &(meteoData->insert( std::pair<std::string, OPAQ::TimeSeries<double> >(
				             parameterId,
							 OPAQ::TimeSeries<double>())).first->second);
	} else {
		out = &(parameterIt->second);
	}

	return out;
}

} /* namespace OPAQ */

OPAQ_REGISTER_PLUGIN(OPAQ::IRCELMeteoProvider);

