#include <iostream>
#include <limits>
#include <cmath>

#include "MLP_FeedForwardModel.h"
#include "feedforwardnet.h"

// define place holder for the neural network input files
const std::string POLLUTANT_PLACEHOLDER   = "%pol%";     // placeholder for the pollutant in config file
const std::string AGGREGATION_PLACEHOLDER = "%aggr%";    // placeholder ofr the aggregation in the config file
const std::string STATION_PLACEHOLDER     = "%station%"; // placeholder for the station in config file
const std::string FCHOR_PLACEHOLDER       = "%fc_hor%";  // idem for forecast horizon
const std::string MODEL_PLACEHOLDER       = "%model%";   // idem for feature vector model

namespace OPAQ {

LOGGER_DEF(OPAQ::MLP_FeedForwardModel);

MLP_FeedForwardModel::MLP_FeedForwardModel() {}
MLP_FeedForwardModel::~MLP_FeedForwardModel() {}


std::string MLP_FeedForwardModel::getFFNetFile( const std::string &pol_name, Aggregation::Type aggr,
	    const std::string &st_name, int fc_hor ) {

	// Building filename...
	std::string fname = this->pattern;
	OPAQ::StringTools::replaceAll( fname, POLLUTANT_PLACEHOLDER, pol_name );
	OPAQ::StringTools::replaceAll( fname, AGGREGATION_PLACEHOLDER, Aggregation::getName(aggr) );
	OPAQ::StringTools::replaceAll( fname, STATION_PLACEHOLDER, st_name );
	OPAQ::StringTools::replaceAll( fname, FCHOR_PLACEHOLDER, std::to_string(fc_hor) );
	OPAQ::StringTools::replaceAll( fname, MODEL_PLACEHOLDER, this->getName() );

  return fname;
}


// need to create a routine which can be re-used by the OVL model without having to make use of the
// run( ) method, as this will require updating the buffers, as we want OVL to be a standalone plugin,
// we should make it independent of the output of the other models --> so run them twice !!!
double MLP_FeedForwardModel::fcValue( const OPAQ::Pollutant& pol, const OPAQ::Station& station,
									  OPAQ::Aggregation::Type aggr, const OPAQ::DateTime& baseTime,
									  const OPAQ::TimeInterval& fc_hor ) {

	// Return the neural network filename, should be implemented in the daugher class
	std::string fname = getFFNetFile( pol.getName(), aggr, station.getName(), fc_hor.getDays() );

	// Read in the network file
	TiXmlDocument nnet_xml( fname.c_str() );
	if ( ! nnet_xml.LoadFile() ) {
		logger->error( "   unable to load ffnet from: " + fname );
		return this->getMissingValue();
	}

	// construct the neural network object
	nnet::feedforwardnet *net;
	try {
		net = new nnet::feedforwardnet( nnet_xml.RootElement() );
	} catch ( const char *msg ) {
		logger->error( "   unable to construct ffnet in " + fname );
		return this->getMissingValue();
	}

	DateTime fcTime = baseTime + fc_hor;

	// construct the input feature vector, output is single pointer value
	// not really nice, but let's leave it for the moment...
	double *input_sample = new double[ net->inputSize() ];
	double *output;

	// call abstract method to generate the sample
	if ( this->makeSample( input_sample, station, pol, baseTime, fcTime, fc_hor ) ) {
		logger->error( "   input sample incomplete, setting missing value" );
		return this->getMissingValue();
	}

	// for ( int ii=0; ii< net->inputSize(); ++ii )
	//   std::cout << "input_sample["<<ii<<"] = " << input_sample[ii] << std::endl;

	// simulate the network
	net->sim( input_sample );

	// retrieve the output & reset the logtransform
	net->getOutput( &output );
	double out = exp(output[0])-1;

	if ( std::isnan(out) || std::isinf( out ) ) out = this->getMissingValue();

	delete [] input_sample;
	delete net;

	return out;
}



/* ============================================================================

     Implementation of the model run method

     ========================================================================== */
void MLP_FeedForwardModel::run() {

	// -- 1. initialization
	logger->debug("MLP_FeedForwardModel " + this->getName() + " run() method called");

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
			logger->trace( " -- basetime: "   + baseTime.dateToString() +
					       ", horizon : day+" + std::to_string(fc_hor) +
					       ", dayN is : "     + fcTime.dateToString() );

			fc.insert( fcTime, fcValue( pol, *station, OPAQ::Aggregation::DayAvg, baseTime, fcHor ) );
		}

		// now we have all the forecast values for this particular station, set the output values...
		buffer->setCurrentModel( this->getName() );
		buffer->setValues( baseTime, fc, station->getName(), pol.getName(), OPAQ::Aggregation::DayAvg );

	} // loop over the stations

}


/* ---------------------------------------------------------------------------------------------- */


/**
 * Computes the mean of the array, taking into account the missing values..
 * just a helper routine
 */
double MLP_FeedForwardModel::mean_missing( const std::vector<double> & list, double noData) {
	double out = 0;
	int noDataCount = 0;
	auto it = list.begin();

	while (it != list.end()) {
		double value = *it++;
		if (value != noData)
			out += value;
		else
			noDataCount++;
	}
	return list.size() - noDataCount > 0 ? out / (list.size() - noDataCount) : noData;
}

/**
 * Computes the max of the array, taking into account the missing values..
 * just a helper routine
 */
double MLP_FeedForwardModel::max_missing( const std::vector<double> & list, double noData) {
	double out = std::numeric_limits<double>::min();
	auto it = list.begin();

	while (it != list.end()) {
		double value = *it++;
		if ( (value != noData) && ( value  > out ) ) out = value;
	}
	return out > std::numeric_limits<double>::min() ? out : noData;
}

/**
 * Computes the min of the array, taking into account the missing values..
 * just a helper routine
 */
double MLP_FeedForwardModel::min_missing( const std::vector<double> & list, double noData) {
	double out = std::numeric_limits<double>::max();
	auto it = list.begin();

	while (it != list.end()) {
		double value = *it++;
		if ( (value != noData) && ( value < out ) ) out = value;
	}
	return out < std::numeric_limits<double>::max() ? out : noData;
}


/**
 * Simple quick 'n dirty debugging routine to print out the parameters
 */
void MLP_FeedForwardModel::printPar( std::string title, const std::vector<double> &x ) {
	std::cout << title;
	for ( auto it = x.begin(); it != x.end() ; ++it ) std::cout << " " << *it;
	std::cout << std::endl;
	return;
}

} /* namespace OPAQ */

