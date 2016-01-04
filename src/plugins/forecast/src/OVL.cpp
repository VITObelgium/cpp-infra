#include <iostream>
#include <limits>
#include <cmath>

#include "OVL.h"
#include "MLP_FeedForwardModel.h"
#include "feedforwardnet.h"

namespace OPAQ {

LOGGER_DEF(OPAQ::OVL);

OVL::OVL() {}
OVL::~OVL() {}

void OVL::configure (TiXmlElement * cnf )
    throw (BadConfigurationException) {

	// here the actual station configuation & RTC configuration should be read, the individual models
	// are already defined... in there respective plugins...
	_conf.clear();

	try {
		// read the mode for which the corresponding tune should be taken
		this->tune_mode = XmlTools::getText(cnf, "tune_mode" );

		// read missing value for this model
		this->missing_value = atoi(XmlTools::getText( cnf, "missing_value" ).c_str() );

		// parse the tunes database & run configuration for OVL
		TiXmlElement *tuneEl = XmlTools::getElementByAttribute( cnf->FirstChildElement( "tunes" ), "tune", "mode", this->tune_mode );
		_parseTunes( tuneEl );

	} catch ( BadConfigurationException & err ) {
		throw BadConfigurationException(err.what());
	}

}


void OVL::_parseTunes( TiXmlElement *lst ) {

	TiXmlElement *polEl = lst->FirstChildElement( "pollutant" );
	while( polEl ) {

		std::string polName = polEl->Attribute( "name" );
		std::string polAggr = polEl->Attribute( "aggr" );

		TiXmlElement *stEl = polEl->FirstChildElement( "station" );
		while ( stEl ) {

			std::string stName = stEl->Attribute( "name" );

			TiXmlElement *modelEl = stEl->FirstChildElement( "model" );
			while( modelEl ) {
				OVL::StationConfig c;
				int fc_hor;

				modelEl->QueryIntAttribute( "fc_hor", &fc_hor );
				modelEl->QueryIntAttribute( "rtc_mode", &(c.rtc_mode) );
				modelEl->QueryIntAttribute( "rtc_param", &(c.rtc_param) );
				c.model_name = modelEl->GetText();

				// std::make_tuple( "pm10", "dayavg", "40ML01", TimeInterval( 1, day ) )
				// inserting using list initializer instead of std::pair()... doesnt work for replacing the std::make_tuple command though
				_conf.insert( { std::make_tuple( polName, polAggr, stName, fc_hor ), c } );

				modelEl = modelEl->NextSiblingElement( "model" );
			}

			stEl = stEl->NextSiblingElement( "station" );
		} // loop over the stations

		polEl = polEl->NextSiblingElement( "pollutant" );
	} // loop over the pollutants

	return;
}


/* ============================================================================

     Implementation of the model run method

     ========================================================================== */
void OVL::run() {

	// -- 1. initialization
	logger->debug("OVL " + this->getName() + " run() method called");

	DateTime baseTime      = getBaseTime();
	Pollutant pol          = getPollutant();
	AQNetwork *net         = getAQNetworkProvider()->getAQNetwork();
	ForecastBuffer *buffer = getBuffer();

	std::vector<Station *> stations = net->getStations();

	OPAQ::Aggregation::Type agg = OPAQ::Aggregation::DayAvg;

	// -- Forecast horizon
	// forecast horizon requested by user is available in abstract model and
	// defined in the configuration file under <forecast><horizon></horizon></forecast>
	// value is given in days, but stored in the TimeInterval format, so have to get days back
	int fcHorMax = getForecastHorizon().getDays();

	// -- 2. loop over the stations
	auto stationIt = stations.begin();

	while ( stationIt != stations.end() ) {
		Station *station = *stationIt++;

		// check if we have a valid meteo id, otherwise skip the station
		if ( station->getMeteoId().length() == 0 ) {
			logger->trace( "Skipping station " + station->getName() + ", no meteo id given" );
			continue;
		} else
			logger->trace("Forecasting station " + station->getName() );

		// store the output in a timeseries object
		OPAQ::TimeSeries<double> fc;
		fc.clear();

		for ( int fc_hor=0; fc_hor <= fcHorMax; fc_hor++ ) {

			OPAQ::TimeInterval fcHor( fc_hor, TimeInterval::Days );
			DateTime fcTime = baseTime + fcHor;

			// lookup the model
			// TODO : put in the correct aggregation, where to get it from ??
			auto modelIt = _conf.find( std::make_tuple( pol.getName(), OPAQ::Aggregation::getName(agg), station->getName(), fc_hor ) );
			if (modelIt == _conf.end() ) {
				// no configuration for this station, returning -9999 or something
				logger->warn(  "Model configuration not found for " + pol.getName() + ", st = " + station->getName() + ", skipping..."   );
				fc.insert( fcTime, this->missing_value );
				continue;
			}


#ifdef DEBUG
			std::cout << "Constructing model " << modelIt->second.model_name << " for " << pol.getName() << ", st = " << station->getName() << ", fc_hor = " << fcHor.getDays()
					  << "rtc_mode = " << modelIt->second.rtc_mode << ", param = " << modelIt->second.rtc_param << std::endl;
#endif

			// get the correct model plugin, we don't have to destroy it as there is only one instance of each component,
			// configuration via the setters...
			MLP_FeedForwardModel *model = ComponentManager::getInstance()->getComponent<MLP_FeedForwardModel>( modelIt->second.model_name );

			// set ins and outs for the model here...
			// this is in fact a small engine...
			model->setBaseTime(baseTime);
			model->setPollutant( pol );
			model->setAQNetworkProvider( getAQNetworkProvider() );
			model->setForecastHorizon( getForecastHorizon() );
			model->setInputProvider( getInputProvider() );
			model->setMeteoProvider( getMeteoProvider() );
			model->setBuffer( getBuffer() );

			// run the model
			double out = model->fcValue( pol, *station, OPAQ::Aggregation::DayAvg, baseTime, fcHor );

			// TODO now handle the RTC !!!




			fc.insert( fcTime, out );
		}

		// TODO set aggregation in the model !!!

		// now we have all the forecast values for this particular station, set the output values...
		buffer->setCurrentModel( this->getName() );
		buffer->setValues( baseTime, fc, station->getName(), pol.getName(), OPAQ::Aggregation::DayAvg );

	} // loop over the stations

}


} /* namespace OPAQ */

OPAQ_REGISTER_PLUGIN(OPAQ::OVL);
