/*
 * RioObsProvider.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "RioObsProvider.h"

namespace OPAQ {

LOGGER_DEF(RioObsProvider);

const std::string RioObsProvider::POLLUTANT_PLACEHOLDER = "%POL%";

RioObsProvider::RioObsProvider() {
	_noData         = -9999;              // RIO observations use -9999 as nodata placeholder
	_timeResolution = TimeInterval(3600); // RIO observations have hourly resolution, per default, can be overwritten !
	_configured     = false;
	_nvalues        = 24;
}

RioObsProvider::~RioObsProvider() {
}

// OPAQ::Component methods
void RioObsProvider::configure( TiXmlElement * cnf )
		throw (BadConfigurationException) {

	// -- parse data file pattern
	TiXmlElement * patternElement = cnf->FirstChildElement( "file_pattern" );
	if (!patternElement)
		throw BadConfigurationException("file_pattern element not found");
	_pattern = patternElement->GetText();

	// -- parse time resolution in minutes, if given...
	TiXmlElement *resEl = cnf->FirstChildElement( "resolution" );
	if ( resEl ) {
		int res = atoi( resEl->GetText() );
		_timeResolution = OPAQ::TimeInterval( res, TimeInterval::Minutes );
		_nvalues = ( 60 * 24 ) / res;
	}

	// -- clear buffer
	_buffer.clear();

	// -- we have a configuration
	_configured = true;
}

// OPAQ::DataProvider methods
TimeInterval RioObsProvider::getTimeResolution() {
	return _timeResolution;
}

double RioObsProvider::getNoData() {
	return _noData;
}

OPAQ::TimeSeries<double> RioObsProvider::getValues( const DateTime& t1, const DateTime& t2,
		  const std::string& stationId, const std::string& pollutantId,
		  OPAQ::Aggregation::Type aggr )  {

	// do some checks
	if ( ! _configured ) throw NotConfiguredException( "Not fully configured" );
	if ( t1 >=  t2 ) throw RunTimeException( "First date is after the second... hmmm" );

	// get pointer to buffered data
	OPAQ::TimeSeries<double> *data = _getTimeSeries( pollutantId, stationId, aggr );
	if ( ! data ) {
		OPAQ::TimeSeries<double> empty; empty.clear();
		return empty;
	}

	//copy the data to the output time series
	OPAQ::TimeSeries<double> out = data->select( t1, t2 );
	return out;
}

/*
 *  old data retrieval for mapping
 *
std::vector<double> RioObsProvider::getValues(const std::string & pollutant,
		const TimeInterval & offset, const ForecastHorizon & forecastHorizon) {
	std::vector<double> out;
	AQNetwork * net = _aqNet->getAQNetwork();
	Pollutant * pol = Config::PollutantManager::getInstance()->find(pollutant);
	std::vector<Station *> * stations = &(net->getStations());
	std::vector<Station *>::iterator it = stations->begin();
	while (it != stations->end()) {
		Station * station = *it++;
		if (AQNetworkTools::stationHasPollutant(station, *pol)) {
			// station has data for the given pollutant
			out.push_back(
					getValues(offset, offset, pollutant, station->getName(),
							forecastHorizon).front());
		} else {
			// station doesn't have data for the given pollutant
			out.push_back(_noData);
		}
	}
	return out;
}
*/

// this returns a reference to where the full array of data is stored for this
// particular combination of pollutant, station and aggregation
OPAQ::TimeSeries<double>* RioObsProvider::_getTimeSeries( const std::string & pollutant,
						   					  	   	   	  const std::string & station,
														  OPAQ::Aggregation::Type aggr ) {

	// first find the pollutant in the map, if we didn't find it, read the file,
	// this should parse the whole set of stations & aggregation times, so no need
	// to re read the file afterwards, only return 0 when data is not found...
	auto it = _buffer.find(pollutant);
	if (it == _buffer.end()) {
		// not found: read data file..
		_readFile( pollutant );
		// and fetch it again
		it = _buffer.find(pollutant);
	}
	if (it == _buffer.end()) return 0;

	// lookup the aggregation
	std::map<OPAQ::Aggregation::Type, std::map<std::string, OPAQ::TimeSeries<double> > > *buffer = &(it->second);
	auto it2 = buffer->find( aggr );
	if ( it2 == buffer->end() ) return 0; // we should have it already once the file is completely read in

	// lookup the station
	std::map<std::string, OPAQ::TimeSeries<double> > *buffer2 = &(it2->second);
	auto it3 = buffer2->find( station );
	if ( it3 == buffer2->end()) return 0;

	return &(it3->second);
}

// parse the file and read in the pollutant //
// no need to specify the aggregation time since we load the whole file in memory and build the
// map in one go...
void RioObsProvider::_readFile( const std::string & pollutant ) {

	// create file name & open file stream
	std::string filename = _pattern;
	StringTools::replaceAll(filename, POLLUTANT_PLACEHOLDER, pollutant);
	std::ifstream file(filename.c_str());

	// time series pointer, to be abused for the different aggregation times etc...
	OPAQ::TimeSeries<double>* ts;



	// read & parse file
	if (file.is_open()) {
		AQNetwork *aqNetwork = _AQNetworkProvider->getAQNetwork();

		std::string line;
		while ( getline(file, line) ) {
			/*
			 * line format:
			 * stationCode YYYYMMDD m1 m8 da hour0 hour1, ..., hour23
			 */
			std::vector<std::string> tokens = StringTools::tokenize(line);

			if ( tokens.size() != (5+_nvalues) )
				throw RunTimeException("format does not match the configuration");


			// only parse stations defined in the network
			if ( AQNetworkTools::containsStation(*aqNetwork, tokens[0] ) ) {
				// only parse lines within the time interval of the buffer
				DateTime begin(atoi(tokens[1].substr(0, 4).c_str()),
						       atoi(tokens[1].substr(4, 2).c_str()),
							   atoi(tokens[1].substr(6, 2).c_str()), 0, 0, 0);

				// get the different aggregations...
				ts = _getOrInitValues( pollutant, OPAQ::Aggregation::Max1h, tokens[0] );
				ts->insert( begin, atof( tokens[2].c_str() ) );  // 3th column is daily max

				ts = _getOrInitValues( pollutant, OPAQ::Aggregation::Max8h, tokens[0] );
				ts->insert( begin, atof( tokens[3].c_str() ) );  // 4th column is max 8h

				ts = _getOrInitValues( pollutant, OPAQ::Aggregation::DayAvg, tokens[0] );
				ts->insert( begin, atof( tokens[4].c_str() ) );  // 5th column is daily avg

				// get the hourly values, no aggregation
				ts = _getOrInitValues( pollutant, OPAQ::Aggregation::None, tokens[0] );
				for ( unsigned int i = 5; i<tokens.size(); i++ ) {
					if ( i > 5 ) begin = begin + _timeResolution;
					ts->insert( begin, atof(tokens[i].c_str() ) );
				}

			}
		}
	} else {
		std::stringstream ss;
		ss << "Failed to open file " << filename;
		logger->warn(ss.str());
	}
}

OPAQ::TimeSeries<double>*  RioObsProvider::_getOrInitValues( const std::string & pollutant,
		OPAQ::Aggregation::Type aggr, const std::string & station ) {


	// buffer:         std::map< std::string, std::map<OPAQ::Aggregation::Type, std::map<std::string, OPAQ::TimeSeries<double> > > >
	// pollutantData : std::map<OPAQ::Aggregation::Type, std::map<std::string, OPAQ::TimeSeries<double> > >
	// aggrData      : std::map<std::string, OPAQ::TimeSeries<double> >


	// get (or create) data array to store the pollutant data
	std::map<OPAQ::Aggregation::Type, std::map<std::string, OPAQ::TimeSeries<double> > > *pollutantData;
	auto it = _buffer.find(pollutant);
	if (it == _buffer.end()) {
		pollutantData =	&(_buffer.insert( std::pair<std::string, std::map<OPAQ::Aggregation::Type, std::map<std::string, OPAQ::TimeSeries<double> > > >(
				pollutant, std::map<OPAQ::Aggregation::Type, std::map<std::string, OPAQ::TimeSeries<double> > >())).first->second);
	} else {
		pollutantData = &(it->second);
	}

	// get of create the aggregation time for this pollutant
	std::map<std::string, OPAQ::TimeSeries<double> > *aggrData;
	auto it2 = pollutantData->find( aggr );
	if ( it2 == pollutantData->end() ) {
		aggrData = &(pollutantData->insert( std::pair<OPAQ::Aggregation::Type, std::map<std::string, OPAQ::TimeSeries<double> > >(
				aggr, std::map<std::string,OPAQ::TimeSeries<double> >())).first->second);
	} else
		aggrData = &(it2->second);

	// get or create the time series object for this pollutant/aggregation time
	OPAQ::TimeSeries<double> * out;
	auto it3 = aggrData->find(station);
	if ( it3 == aggrData->end() ) {
		// vector not found: init it with all missing data
		out = &(aggrData->insert( std::pair<std::string, OPAQ::TimeSeries<double> >(station, OPAQ::TimeSeries<double>())).first->second);
	} else {
		out = &(it3->second);
	}

	return out;
}

} /* namespace OPAQ */

OPAQ_REGISTER_PLUGIN(OPAQ::RioObsProvider);
