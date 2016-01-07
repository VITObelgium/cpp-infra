#include <iostream>
#include <limits>
#include <cmath>

#include "OVL.h"
#include "MLP_FeedForwardModel.h"
#include "feedforwardnet.h"

namespace OPAQ {

LOGGER_DEF(OPAQ::OVL);

OVL::OVL() :
	missing_value(-9999),
	output_raw(false) {
}

OVL::~OVL() {}

/**
 * Helper function to compute the weighted exponetial weights. This is based upon
 *
 * \sum_{i=0}^{N-1} r^n = \frac{1-r^N}{1-r}
 *
 * and is used in mode 2 of the RTC where the hindcast errors are weighted exponentially, given larger
 * weight to the more recent errors.
 *
 * The shape of the exponential
 * weight is determined by the parameter 'param' (> 0). For  smaller values this
 * parameter will give more weight to the recent errors, for param -> inf this will
 * distribute the weight more equally across the hind cast. The limit of param -> infinity is
 * mode 1, the simple average... THe table below gives some indication of the weight of the
 * first and second most recent errors :
 *
 *      param | weights...
 *      ------|--------------------------
 *        0   | 1.00 0.00 0.00 0.00 0.00
 *        1   | 0.50 0.25 0.13 0.06 0.03
 *        2   | 0.33 0.22 0.15 0.10 0.07
 *        3   | 0.25 0.18 0.14 0.11 0.08
 *
 *  \param p is the parameter p for the exponential (given in the XML station config)
 *  \param i is the index, where i = 0 means yesterday (the first index in the array of
 *           hind cast errors
 *  \param n is the length of the hindcast window
 */
double _wexp( int i, int n, int p ) {
	double lambda = static_cast<double>(p)/(1.+static_cast<double>(p));
	return (1.-lambda)*std::pow(lambda,i)/(1.-std::pow(lambda,n));
}


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

		// read hind cast period
		this->hindcast = OPAQ::TimeInterval( atoi(XmlTools::getText( cnf, "hindcast" ).c_str() ), OPAQ::TimeInterval::Days );

		// parse the tunes database & run configuration for OVL
		parseTuneList( cnf->FirstChildElement( "tunes" ) );

	} catch ( BadConfigurationException & err ) {
		throw BadConfigurationException(err.what());
	}

	// get output mode
	try {
		std::string s = XmlTools::getText( cnf, "output_raw");
		std::transform( s.begin(), s.end(), s.begin(), ::tolower );
		if ( !s.compare( "enable" ) || !s.compare( "true" ) || !s.compare( "yes" ) ) this->output_raw = true;
	} catch ( ... ) {
		this->output_raw = false;
	}


}

void OVL::parseTuneElement( TiXmlElement *tuneEl ) {

	std::string tunePol, tuneAggr, tuneMode;
	if ( tuneEl->QueryStringAttribute("pollutant", &tunePol) != TIXML_SUCCESS ) throw BadConfigurationException("pollutant not found in tune");
	if ( tuneEl->QueryStringAttribute("aggr", &tuneAggr) != TIXML_SUCCESS ) throw BadConfigurationException("aggr not found in tune");
	if ( tuneEl->QueryStringAttribute("mode", &tuneMode) != TIXML_SUCCESS ) throw BadConfigurationException("mode not found in tune");

	TiXmlElement *stEl = tuneEl->FirstChildElement( "station" );
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

			// inserting using list initializer instead of std::pair()... doesnt work for replacing the std::make_tuple command though
			_conf.insert( { std::make_tuple( tunePol, Aggregation::fromString( tuneAggr ), tuneMode, stName, fc_hor ), c } );

			modelEl = modelEl->NextSiblingElement( "model" );
		}

		stEl = stEl->NextSiblingElement( "station" );
	} // loop over the stations
}


void OVL::parseTuneList( TiXmlElement *lst ) {

	TiXmlDocument refDoc;
	TiXmlElement *tuneEl = lst->FirstChildElement( "tune" );
	while( tuneEl ) {
		std::string ref;
		if ( tuneEl->QueryStringAttribute("ref", &ref) == TIXML_SUCCESS ) {
			// ref attribute found
			if (FileTools::exists(ref)) {
				// file found
				if (!refDoc.LoadFile(ref)) {
					std::stringstream ss;
					ss << "Failed to load file in ref attribute: " << ref;
					throw ElementNotFoundException(ss.str());
				}
				TiXmlElement* fileElement = refDoc.FirstChildElement( "tune" );
				if (!fileElement) {
					std::stringstream ss;
					ss << "File in ref attribute (" << ref
						<< ") does not have 'tune' as root element";
					throw ElementNotFoundException(ss.str());
				}
				parseTuneElement( fileElement );

			} else {
				std::stringstream ss;
				ss << "File in ref attribute '" << ref << "' not found.";
				throw ElementNotFoundException(ss.str());
			}

		} else {
			parseTuneElement( tuneEl );
		}

		tuneEl = tuneEl->NextSiblingElement( "tune" );
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
	Aggregation::Type aggr = getAggregation();
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

			// lookup the station configuration
			auto stIt = _conf.find( std::make_tuple( pol.getName(), aggr, this->tune_mode, station->getName(), fc_hor ) );
			if (stIt == _conf.end() ) {
				// no configuration for this station, returning -9999 or something
				logger->warn(  "Model configuration not found for " + pol.getName() + ", st = " + station->getName() + ", skipping..."   );
				fc.insert( fcTime, this->missing_value );
				continue;
			}

			// get a pointer to the station configuration
			StationConfig *cf = &(stIt->second);

#ifdef DEBUG
			std::cout << "Constructing model " << cf->model_name << " for " << pol.getName() << ", st = " << station->getName() << ", fc_hor = " << fcHor.getDays()
					  << "rtc_mode = " << cf->rtc_mode << ", param = " << cf->rtc_param << std::endl;
#endif

			// get the correct model plugin, we don't have to destroy it as there is only one instance of each component,
			// configuration via the setters...
			MLP_FeedForwardModel *model = ComponentManager::getInstance()->getComponent<MLP_FeedForwardModel>( cf->model_name );

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
			double out = model->fcValue( pol, *station, aggr, baseTime, fcHor );


			if ( output_raw ) {
				// DOCUMENT THIS !!!!

				// TODO : enable this piece of code only when we are only running OVL
				// Note that this can mean that the output buffer is not complete and the models are not necessarily in the correct order...

				// now we have all the forecast values for this particular station, set the output values...
				OPAQ::TimeSeries<double> raw_fc;
				raw_fc.clear();
				raw_fc.insert(fcTime,out);
				buffer->setCurrentModel( model->getName() );
				buffer->setValues( baseTime, raw_fc, station->getName(), pol.getName(), aggr );
			}

			// now handle the RTC, if the mode is larger than 0, otherwise we already have out output !!!
			// get the historic forecasts
			if ( cf->rtc_mode > 0 ) {

				// get the hind cast for the forecast times between yesterday & the hindcast
				OPAQ::DateTime t1 = baseTime - hindcast;
				OPAQ::DateTime t2 = baseTime - OPAQ::TimeInterval( 1, OPAQ::TimeInterval::Days );
#ifdef DEBUG
				std::cout << "*** calling getValues, OVL baseTime is " << baseTime << std::endl;
#endif
				buffer->setCurrentModel( model->getName() );
				OPAQ::TimeSeries<double> fc_hindcast = buffer->getValues( fcHor, t1, t2, station->getName(), pol.getName(), aggr );

#ifdef DEBUG
				std::cout << "forecast hindcast : " << std::endl;
				std::cout << fc_hindcast << std::endl;
#endif

				// get the observed values from the input provider
				// we could also implement it in such way to get them back from the forecast buffer...
				// here a user should simply make sure we have enough data available in the data provider...
				OPAQ::TimeSeries<double> obs_hindcast = getInputProvider()->getValues( t1, t2, station->getName(), pol.getName(), aggr );

#ifdef DEBUG
				std::cout << "observation hindcast : " << std::endl;
				std::cout << obs_hindcast << std::endl;
#endif

				// check if the timeseries are consistent !
				if ( ! fc_hindcast.isConsistent( obs_hindcast ) )
					throw RunTimeException( "foreacst & hindcast timeseries are not consistent..." );

				double fc_err = 0.;
				// run the real time correction scheme, let's do it this way for the moment,
				// later on we can add a separate class or even plugin for this...
				switch (cf->rtc_mode) {
					case 1:
						{	// compute the average error in the hind cast period
							int n = 0;
							for ( unsigned int i = 0; i < fc_hindcast.size(); i++ ) {
								if ( fabs( fc_hindcast.value(i) - buffer->getNoData() ) > 1.e-6 &&
									 fabs( obs_hindcast.value(i) - getInputProvider()->getNoData() ) > 1.e-6 ) {

									fc_err += ( fc_hindcast.value(i) - obs_hindcast.value(i) );

									n++;
								}
							}
							if ( n > 0 ) fc_err /= n;
						}
						break;

					case 2:
						{	// compute a weighted error in the hindcast period
							// TODO here we don't take the weights of the missing values in either
							//      obs / fc arrays into account... maybe a fix for later (was never so in OVL)
							for ( unsigned int i = 0; i < fc_hindcast.size(); i++ ) {
								if ( fabs( fc_hindcast.value(i) - buffer->getNoData() ) > 1.e-6 &&
								     fabs( obs_hindcast.value(i) - getInputProvider()->getNoData() ) > 1.e-6 ) {

									fc_err += _wexp( i, fc_hindcast.size(), cf->rtc_param ) * ( fc_hindcast.value(i) - obs_hindcast.value(i) );

								}
							}
						}
						break;

					default:
						throw BadConfigurationException( "Invalid real time correction mode : " + std::to_string( cf->rtc_mode ) );
						break;
				}

				// perform correction
				out = out - fc_err;
			}

			// insert the final forecast...
			fc.insert( fcTime, out );
		}

		// now we have all the forecast values for this particular station, set the output values...
		buffer->setCurrentModel( this->getName() );
		buffer->setValues( baseTime, fc, station->getName(), pol.getName(), aggr );

	} // loop over the stations

}


} /* namespace OPAQ */

OPAQ_REGISTER_PLUGIN(OPAQ::OVL);
