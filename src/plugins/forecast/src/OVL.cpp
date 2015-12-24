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

			// import the model plugin, but cast it as a MLP_FeedForwardModel... instead of a Model is that possible ??? YES !!!! moehaha !!
			std::string modelName = "OVL_model2_9UT"; // has to be the component name in the xml !!!
			MLP_FeedForwardModel *model = ComponentManager::getInstance()->getComponent<MLP_FeedForwardModel>( modelName );

			double out = model->fcValue( pol, *station, OPAQ::Aggregation::DayAvg, baseTime, fcHor );

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
