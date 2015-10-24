#include <string>
#include "math.h"
#include "feedforwardnet.h"

#include "FeedForwardModel.h"

#include "FeatureGenerator.h"
#include "OVL_model_1_7CST.h"
#include "OVL_model_2_7CST.h"
#include "OVL_model_3_7CST.h"

void print_par( std::string title, const std::vector<double> &x ) {
  std::cout << title;
  for ( std::vector<double>::const_iterator it = x.begin(); it != x.end() ; ++it ) std::cout << " " << *it;
  std::cout << std::endl;
  return;
}

int dow(int y, int m, int d)
{
  static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  y -= m < 3;
  return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

namespace OPAQ {

  LOGGER_DEF(OPAQ::FeedForwardModel);

#define __MISSING_VALUE__ -9999
#define pi 3.141592654

  // define placeholders for the neural network input files
  const std::string POLLUTANT_PLACEHOLDER = "%pol%";     // placeholder for the pollutant in config file
  const std::string STATION_PLACEHOLDER   = "%station%"; // placeholder for the station in config file
  const std::string FCHOR_PLACEHOLDER     = "%fc_hor%";   // idem for forecast horizon
  const std::string MODEL_PLACEHOLDER     = "%model%";   // idem for feature vector model

  // a list of meteo variable names relevant for this model
  // we should acutally better put this in the config file and allow the user
  // to configure the ff model feature vector from the configuration
  const std::string p_t2m     = "P01"; // 2m temperature
  const std::string p_wsp10m  = "P03"; // wind speed 10 m
  const std::string p_wdir10m = "P04"; // wind direction 10 m 
  const std::string p_blh     = "P07"; // boundary layer height
  const std::string p_cc      = "P12"; // total cloud cover

  FeedForwardModel::FeedForwardModel() {}

  FeedForwardModel::~FeedForwardModel() {}

  /* ============================================================================
   
     Implementation of the configure method
     
     ========================================================================== */
  void FeedForwardModel::configure (TiXmlElement * cnf ) throw (BadConfigurationException) {

    /* ------------------------------------------------------------------------
       Getting the configation options
       --------------------------------------------------------------------- */
    logger->trace("Configuring feedforward neural network model");
    try {

      // read the path to the architecture files
      this->pattern = XmlTools::getText(cnf, "ffnetfile_pattern" );

      // read the maximum number of forecast days
      this->max_fc_hor = atoi(XmlTools::getText( cnf, "max_fc_hor" ).c_str() );
      
      // read morning aggregation 
      this->mor_agg = atoi(XmlTools::getText( cnf, "mor_agg" ).c_str() );

      // get the requested tune and find the tunelist
      _tuneSel = XmlTools::getText(cnf, "tune_selection" );
      _tuneList = XmlTools::getElement( cnf, "tunelist" );
      if ( ! _tuneList ) throw( BadConfigurationException( "No tunelist discovered in config" ) ); 

    } catch (ElementNotFoundException & e) {
      throw BadConfigurationException(e.what());
    }
    
  }
  

  void FeedForwardModel::_selectTune( const Pollutant & pol ) 
    throw( BadConfigurationException ) {
    
    std::string name;
    bool found_tune = false;
    TiXmlElement *el = _tuneList->FirstChildElement( "tune");
    while( el ) {
      if ( el->QueryStringAttribute( "name", &name ) != TIXML_SUCCESS ) 
	throw( BadConfigurationException("Error in OVL tune configuration, no tune name given in list"));
      
      if ( name == _tuneSel ) {
	// we have the tune, now locate the pollutant
	TiXmlElement *polEl = el->FirstChildElement( "pollutant" );
	while( polEl ) {
	  std::string polName;
	  if ( polEl->QueryStringAttribute( "name", &polName ) != TIXML_SUCCESS )
	    throw( BadConfigurationException("Error in OVL tune configuration, no name for pollutant"));
	  
	  std::cout << "trying : " << polName << "want : " << pol.getName() << std::endl;

	  if ( polName == pol.getName() ) {
	    // we have found the pollutant configuration in this tune, let's parse it !

	    TiXmlElement *stEl = polEl->FirstChildElement( "station" );
	    while( stEl ) {
	      std::string stName;
	      if ( stEl->QueryStringAttribute( "name", &stName ) != TIXML_SUCCESS )
		throw( BadConfigurationException("No name for station given in OVL tune config"));
	      
	      Station *s = getAQNetworkProvider()->getAQNetwork()->findStation( stName );
	      if ( ! s ) throw( BadConfigurationException("Station " + stName + 
							  " not found in network in OVL tune config"));
	      OVL::Tune t;
	      t.setFeaturesName( XmlTools::getText(stEl, "features" ) );

	      // insert the tune for this station into the tunemap
	      _tuneConfig.insert(std::pair<Station*,OVL::Tune>(s,t));

	      stEl = stEl->NextSiblingElement("station");
	    }

	    // we have found a tune...
	    logger->trace("Tune succesfully discovered in configuration...");
	    found_tune = true;
	    break;
	  }

	  polEl = polEl->NextSiblingElement( "pollutant" );
	}
	
      }
      
      el = el->NextSiblingElement( "tune" );
    }

    if ( ! found_tune ) throw( BadConfigurationException( "Cannot find appropriate OVL tune : " + _tuneSel ));

    // dump the tune
    std::vector<Station*> st_list = getAQNetworkProvider()->getAQNetwork()->getStations();
    for ( std::vector<Station*>::iterator it = st_list.begin(); it != st_list.end(); it++ ) {
      std::cout << "Station " << (*it)->getName() << " tune : ";
      
      std::map<Station*,OVL::Tune>::iterator itTune = _tuneConfig.find( *it );
      if ( itTune != _tuneConfig.end() ) {
	std::cout << (itTune->second).getFeaturesName();
      } else std::cout << "not found !";

      std::cout << std::endl;
    }

    return;
  }


  /* ============================================================================
   
     Implementation of the model run method
    
     ========================================================================== */
  void FeedForwardModel::run() {

    // -- 1. initialization
    logger->debug("FeedForward Model run() method called");
    logger->trace("getting base time and pollutant");

    DateTime baseTime   = getBaseTime();
    Pollutant pol       = getPollutant();
    
    logger->trace("Getting input data providers");
    DataProvider *obs   = getInputProvider();
    DataProvider *meteo = getMeteoProvider();

    // not used in this demo, but we can fetch them if we need them:
    // Depending on a configuration option in the config file...
    // try {
    //  DataProvider * historical = getHistoricalForecastsProvider();
    // } catch (NullPointerException &e) {
    //  logger->trace("Got null pointer for historical forecasts: OK");
    // }
    
    logger->trace("Getting air quality network");
    AQNetwork *net = getAQNetworkProvider()->getAQNetwork();
    
    logger->trace("Getting output data store");
    DataStore *output = getOutputStore();

    logger->trace("Fetching stations from air quality network");
    std::vector<Station *> stations = net->getStations();

    // -- 2 Select the appropriate tune for this pollutant
    _selectTune( pol );


    // -- 3. loop over the stations
    std::cout << "Iterating over stations..." << std::endl;
    std::vector<Station *>::iterator stationIt = stations.begin();
    while (stationIt != stations.end()) {
      Station *station = *stationIt++;

      logger->trace(" Forecasting station " + station->getName() );

      std::vector<double> fc_value;
      std::vector<OPAQ::ForecastHorizon> fc_time;

      // reset the output vectors
      fc_value.clear();
      fc_time.clear();

      for ( int fc_hor=0; fc_hor <= max_fc_hor; fc_hor++ ) {

	// Setting correct timing for this particular forecast
	DateTime fcTime = baseTime;
	fcTime.addDays( fc_hor );
	logger->trace( " -- basetime: " + baseTime.toString() + 
		       ", horizon : day+" + std::to_string(fc_hor) + 
		       ", dayN is : " + fcTime.toString() );

	OPAQ::ForecastHorizon fcHor(fc_hor*24);

	// forecast horizons are in hours, so here we do daily forecats, so we 
	// need to mutiply the forecast horizon by 24...
	fc_time.push_back( fcHor );


	// get the feature vector model
	std::string featVecModel;
	std::map<Station*,OVL::Tune>::iterator itTune = _tuneConfig.find( station );
	if ( itTune != _tuneConfig.end() ) {
	  featVecModel = (itTune->second).getFeaturesName();
	} else {
	  logger->trace( "No tune configuration for " + station->getName() + ", skipping foreacst..." );
	}
	
	std::string fname = this->pattern;
	OPAQ::StringTools::replaceAll( fname, POLLUTANT_PLACEHOLDER, pol.getName() );
	OPAQ::StringTools::replaceAll( fname, STATION_PLACEHOLDER, station->getName() );
	OPAQ::StringTools::replaceAll( fname, FCHOR_PLACEHOLDER, std::to_string(fc_hor) );
	OPAQ::StringTools::replaceAll( fname, MODEL_PLACEHOLDER, featVecModel );

	logger->trace( " -- loading neural network: " + fname );

	// Read in the network file
	TiXmlDocument nnet_xml( fname.c_str() );
	if ( ! nnet_xml.LoadFile() ) {
	  logger->error( "Cannot load neural network from: " + fname );
	  fc_value.push_back( __MISSING_VALUE__ );
	  continue;
	}

	// construct the neural network object
	nnet::feedforwardnet *net;
	try {
	  net = new nnet::feedforwardnet( nnet_xml.RootElement() );
	} catch ( const char *msg ) {
	  logger->error( "Error running neural network" );
	  fc_value.push_back( __MISSING_VALUE__ );
	  continue;
	}
	//std::cout << "Loaded network : " << std::endl;
	//std::cout << *net << std::endl;

	// construct the input feature fector, we can probably make this nicer via a factory
	OVL::FeatureGenerator *fGun;
	if ( featVecModel == "model-1-7CST" ) {
	  fGun = new OVL::model_1_7CST( meteo, obs );
	} else if ( featVecModel == "model-2-7CST" )  {
	  fGun = new OVL::model_2_7CST( meteo, obs );
	} else if ( featVecModel == "model-3-7CST" )  {
	  fGun = new OVL::model_3_7CST( meteo, obs );
	} else {
	  logger->error( "This feature vector model is not implemented" );
	  delete net;
	  continue;
	}

	double *input_sample = new double[fGun->size()];

	// call abstract method...
	fGun->makeSample( input_sample, station, &pol, baseTime, fcHor ); 

	for ( int ii=0; ii<fGun->size(); ++ii ) 
	  std::cout << "input_sample["<<ii<<"] = " << input_sample[ii] << std::endl;

	// -----------------------
	// meteo input
	// -----------------------
	// we shift the meteo basetime to the forecast timebase, i.e. dayN ! 
	meteo->setBaseTime( fcTime );

	// we have a good sample, at least we assume...
	bool have_sample = true;

	// BLH for dayN, offsets relative from fcTime (set by setBaseTime in the meteo provider)
	TimeInterval dayNbegin(0);
	TimeInterval dayNend(18*3600); // 1x meteto TimeResultion aftrekken : - getTimeResolution();
	std::vector<double> blh  = meteo->getValues( dayNbegin, dayNend, station->getMeteoId(), p_blh );
	std::vector<double> cc   = meteo->getValues( dayNbegin, dayNend, station->getMeteoId(), p_cc );


	TimeInterval dayNm1Half(-12*3600);  // dayN-1 : 12:00 and 18:00
	TimeInterval dayNHalf(6*3600);      // dayN   : 00:00 and 06:00
	std::vector<double> wsp  = meteo->getValues( dayNm1Half, dayNHalf, station->getMeteoId(), p_wsp10m );
	std::vector<double> wdir = meteo->getValues( dayNm1Half, dayNHalf, station->getMeteoId(), p_wdir10m );
	
	// calculate the windspeed u and v components from the wsp and wdir
	std::vector<double> u,v;
	u.resize(wsp.size());
	v.resize(wsp.size());
	for ( int ii=0; ii< u.size(); ii++ ) {
	  if ( ( wdir[ii] == meteo->getNoData( p_wdir10m ) ) || 
	       ( wsp[ii]  == meteo->getNoData( p_wsp10m ) ) ) {
	    logger->warn( "missing wind information in meteo, skipping... " );
	    have_sample = false;
	  } else {
	    u[ii] = wsp[ii] * cos(pi*wdir[ii]/180);
	    v[ii] = wsp[ii] * sin(pi*wdir[ii]/180);
	  }
	}

	print_par( "BLH values  : ", blh );
	print_par( "Cloud cover : ", cc  );
	print_par( "Wind speed  : ", wsp );
	print_par( "Wind dir    : ", wdir );
	print_par( "u           : ", u );
	print_par( "v           : ", v );


	// -----------------------
	// get the concentrations
	// -----------------------
	// basetime for the observation data provider is not changes
	TimeInterval begYest(-24*3600);
	TimeInterval endYest(-3600); //= - obs->getTimeResolution(); 
                                     // 1x meteto TimeResultion aftrekken : - getTimeResolution();
	std::vector<double> xx_yest = obs->getValues( begYest, endYest, pol.getName(), station->getName() );

	TimeInterval begMor(0);
	TimeInterval endMor((this->mor_agg-1)*3600); // met mor_agg uur of eentje aftrekken ????
	std::vector<double> xx_morn = obs->getValues( begMor, endMor, pol.getName(), station->getName() );

	print_par( "Retrieved pollutant info yesterday : ", xx_yest );
	print_par( "Retrieved pollutant info morning   : ", xx_morn );

	// -----------------------
	// get the weekday
	// -----------------------
	int weekend = 0;
	if ( ( fcTime.getDayOfWeek() == 0 ) || ( fcTime.getDayOfWeek() == 6 ) ) weekend = 1;

	std::cout << "year : " << fcTime.getYear() << std::endl;
	std::cout << "mon : " << fcTime.getMonth() << std::endl;
	std::cout << "day : " << fcTime.getDay() << std::endl;
	std::cout << "day of week : " << fcTime.getDayOfWeek() << std::endl;
	std::cout << "day of week : " << dow( fcTime.getYear() , fcTime.getMonth(), fcTime.getDay()  ) << std::endl;
	// dow = (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7 

	// -----------------------
	// build sample
	// -----------------------

	// add some more checks for missing values,  if sample not complete... either estimate value
	// from climatology ?? or don't run forecast...

	double sample[7];
	double *output;

	sample[0] = log(1 + mean_missing( xx_morn, obs->getNoData() ) );       // PMMOR
	sample[1] = log(1 + mean_missing( xx_yest, obs->getNoData() ) );     // PMYEST
	sample[2] = log(1 + mean_missing(blh,meteo->getNoData( p_blh ) ) );  // BLH
	sample[3] = log(1 + mean_missing(cc, meteo->getNoData( p_cc  ) ) );  // MCC
	sample[4] = mean_missing( u, -999 ); // wdir x
	sample[5] = mean_missing( v, -999 ); // wdir y
	sample[6] = weekend;                 // weekend
	

	// simulate the network
	net->sim( sample );
	// retrieve the output
	net->getOutput( &output );
	// store the output
	fc_value.push_back( exp(output[0])-1 );

	logger->trace( " -- forecast value: " + std::to_string(exp(output[0])-1) );
	//printf( " Output       : %f\n", exp(output[0])-1 );

	delete input_sample;
	delete fGun;
	delete net;
      }

      // now we have all the forecast values for this particular station, set the output values...
      logger->trace("writing ffnet forecast output for station " + station->getName() );
      output->setValues( fc_value, fc_time, pol.getName(), station->getName() );

    } // loop over the stations

  }
  
  double FeedForwardModel::mean_missing(std::vector<double> & list, double noData) {
    double out = 0;
    int noDataCount = 0;
    std::vector<double>::iterator it = list.begin();
    while (it != list.end()) {
      double value = *it++;
      if (value != noData)
	out += value;
      else
	noDataCount++;
    }
    return list.size() - noDataCount > 0 ? out / (list.size() - noDataCount) : out;
  }
  
} /* namespace OPAQ */


OPAQ_REGISTER_PLUGIN(OPAQ::FeedForwardModel);
