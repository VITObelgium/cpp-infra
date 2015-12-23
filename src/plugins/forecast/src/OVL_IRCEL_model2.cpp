#include "OVL_IRCEL_model2.h"

// a list of meteo variable names relevant for this model
// we should acutally better put this in the config file and allow the user
// to configure the ff model feature vector from the configuration
const std::string p_t2m     = "P01"; // 2m temperature
const std::string p_wsp10m  = "P03"; // wind speed 10 m
const std::string p_wdir10m = "P04"; // wind direction 10 m 
const std::string p_blh     = "P07"; // boundary layer height
const std::string p_cc      = "P12"; // total cloud cover

  
// define placeholders for the neural network input files
const std::string POLLUTANT_PLACEHOLDER = "%pol%";     // placeholder for the pollutant in config file
const std::string STATION_PLACEHOLDER   = "%station%"; // placeholder for the station in config file
const std::string FCHOR_PLACEHOLDER     = "%fc_hor%";   // idem for forecast horizon
const std::string MODEL_PLACEHOLDER     = "%model%";   // idem for feature vector model
const std::string MCID_PLACEHOLDER      = "%mcid%";   //  idem for the morning Id : 9UT, 7CST etc...

namespace OPAQ {

  OVL_IRCEL_model2::OVL_IRCEL_model2() {
    missing_value = -9999; // set default missing value, overwritting in xml config
    mor_agg       = 9;     // default run hour...
  }

  OVL_IRCEL_model2::~OVL_IRCEL_model2() {};

  /* ============================================================================
     Implementation of the configure method
     ========================================================================== */
  void OVL_IRCEL_model2::configure (TiXmlElement * cnf ) 
    throw (BadConfigurationException) {
    
    try {
      
      // read the path to the architecture files
      this->pattern = XmlTools::getText(cnf, "ffnetfile_pattern" );
      
      // read morning aggregation 
      this->mor_agg = atoi(XmlTools::getText( cnf, "mor_agg" ).c_str() );
      
      // read matching id
      this->mcid = XmlTools::getText( cnf, "mcid");

      // read missing value for this model
      this->missing_value = atoi(XmlTools::getText( cnf, "missing_value" ).c_str() );

    } catch (ElementNotFoundException & e) {
      throw BadConfigurationException(e.what());
    }

    // now read the real time correction configuration
    

    
  }

  /* ============================================================================
     Constructs the filename for the neural network parameters
     ========================================================================== */
  std::string OVL_IRCEL_model2::getFFNetFile( const std::string &pol_name, 
					      const std::string &st_name, 
					      int fc_hor ) {

    // Building filename... 
    std::string fname = this->pattern;
    OPAQ::StringTools::replaceAll( fname, POLLUTANT_PLACEHOLDER, pol_name );
    OPAQ::StringTools::replaceAll( fname, STATION_PLACEHOLDER, st_name );
    OPAQ::StringTools::replaceAll( fname, FCHOR_PLACEHOLDER, std::to_string(fc_hor) );
    OPAQ::StringTools::replaceAll( fname, MODEL_PLACEHOLDER, this->getName() );
    OPAQ::StringTools::replaceAll( fname, MCID_PLACEHOLDER, this->mcid );

    return fname;
  }


  
  /* ============================================================================
     Construct sample for the OVL_IRCEL_model2 configuration
     ========================================================================== */
  int OVL_IRCEL_model2::makeSample( double *sample,
		  	  	  	  	  	  	    OPAQ::Station *st,
									OPAQ::Pollutant *pol,
				    				const OPAQ::DateTime &baseTime,
									const OPAQ::DateTime &fcTime,
									const OPAQ::TimeInterval &fc_hor ) {
    
    int have_sample = 0; // return code, 0 for success
    OPAQ::DateTime t1, t2;

    // -----------------------
    // Getting data providers --> stored in main model...
    // -----------------------
    DataProvider *obs    = getInputProvider();
    MeteoProvider *meteo = getMeteoProvider();
    
    // -----------------------
    // Get the meteo input
    // -----------------------
    // BLH for dayN, offsets relative from fcTime (set by setBaseTime in the meteo provider)
    t1 = fcTime + OPAQ::TimeInterval(0);
    t2 = fcTime + OPAQ::TimeInterval(24*3600) - meteo->getTimeResolution();

    TimeSeries<double> blh  = meteo->getValues( t1, t2, st->getMeteoId(), p_blh );
    TimeSeries<double> cc   = meteo->getValues( t1, t2, st->getMeteoId(), p_cc );
    
    t1 = fcTime - TimeInterval(12*3600); // dayN-1 : 12:00 to end, including 12:00
    t2 = fcTime + TimeInterval(12*3600) - meteo->getTimeResolution(); // day N : 00:00 to 12:00 (excluding)
    
    TimeSeries<double> wsp  = meteo->getValues( t1, t2, st->getMeteoId(), p_wsp10m );
    TimeSeries<double> wdir = meteo->getValues( t1, t2, st->getMeteoId(), p_wdir10m );
    

    // TODO do we have enough meteo info ?
    std::cout << "TODO : check what is returned by the meteo getter" << std::endl;

    // calculate the windspeed u and v components from the wsp and wdir
    std::vector<double> u,v;

    u.resize(wsp.size());
    v.resize(wsp.size());

    for ( unsigned int ii=0; ii< u.size(); ii++ ) {
    	if ( ( wdir.value(ii) == meteo->getNoData( p_wdir10m ) ) ||
    		 ( wsp.value(ii)  == meteo->getNoData( p_wsp10m ) ) ) {
    		// increase the error count
    		have_sample++;
      } else {
    	  u[ii] = wsp.value(ii) * cos( OPAQ::Math::Pi * wdir.value(ii) / 180. );
    	  v[ii] = wsp.value(ii) * sin( OPAQ::Math::Pi * wdir.value(ii) / 180. );
      }
    }

    /*
    printPar( "BLH values  : ", blh );
    printPar( "Cloud cover : ", cc  );
    printPar( "Wind speed  : ", wsp );
    printPar( "Wind dir    : ", wdir );
    printPar( "u           : ", u );
    printPar( "v           : ", v );
    */
    
    // -----------------------
    // get the concentrations
    // -----------------------
    // basetime for the observation data provider is not changes
    t1 = DateTimeTools::floor( baseTime, DateTimeTools::FIELD_DAY ) - TimeInterval( 1, TimeInterval::Days );
    t2 = DateTimeTools::floor( baseTime, DateTimeTools::FIELD_DAY ) - obs->getTimeResolution();
    // 1x meteto TimeResultion aftrekken : - getTimeResolution();
    OPAQ::TimeSeries<double> xx_yest = obs->getValues( t1, t2, st->getName(), pol->getName() );
    
    
    t1 = DateTimeTools::floor( baseTime, DateTimeTools::FIELD_DAY );
    t2 = t1 + OPAQ::TimeInterval( this->mor_agg-1, TimeInterval::Hours );// met mor_agg uur of eentje aftrekken ????

    OPAQ::TimeSeries<double> xx_morn = obs->getValues(t1, t2, st->getName(), pol->getName() ); // no aggregation

    /*
    printPar( "Retrieved pollutant info yesterday : ", xx_yest );
    printPar( "Retrieved pollutant info morning   : ", xx_morn );
    */

    // ---------------------------------
    // get the weekday : week/weekend ?
    // --------------------------------
    int weekend = 0;
    if ( ( fcTime.getDayOfWeek() == 0 ) || ( fcTime.getDayOfWeek() == 6 ) ) weekend = 1;
    

    // -----------------------
    // build sample
    // -----------------------
    
    // add some more checks for missing values,  if sample not complete... either estimate value
    // from climatology ?? or don't run forecast...
    sample[0] = log(1 + mean_missing( xx_morn.values(), obs->getNoData() ) );     // PMMOR
    sample[1] = log(1 + mean_missing( xx_yest.values(), obs->getNoData() ) );     // PMYEST
    sample[2] = log(1 + mean_missing(blh.values(),meteo->getNoData( p_blh ) ) );  // BLH
    sample[3] = log(1 + mean_missing(cc.values(), meteo->getNoData( p_cc  ) ) );  // MCC
    sample[4] = mean_missing( u, -999 ); // wdir x
    sample[5] = mean_missing( v, -999 ); // wdir y
    sample[6] = weekend;                 // weekend
    
    return have_sample;
  }
  
}

OPAQ_REGISTER_PLUGIN(OPAQ::OVL_IRCEL_model2);
