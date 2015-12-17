#include <opaq.h>
#include "OVL_IRCEL_model1.h"

#define pi 3.141592654

void print_par( std::string title, const std::vector<double> &x ) {
  std::cout << title;
  for ( std::vector<double>::const_iterator it = x.begin(); it != x.end() ; ++it ) std::cout << " " << *it;
  std::cout << std::endl;
  return;
}

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

  OVL_IRCEL_model1::OVL_IRCEL_model1() {
    missing_value = -9999; // set default missing value, overwritting in xml config
  }

  OVL_IRCEL_model1::~OVL_IRCEL_model1() {};


  /* ============================================================================
     Implementation of the configure method
     ========================================================================== */
  void OVL_IRCEL_model1::configure (TiXmlElement * cnf ) 
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
    
  }

  /* ============================================================================
     Constructs the filename for the neural network parameters
     ========================================================================== */
  std::string OVL_IRCEL_model1::getFFNetFile( const std::string &pol_name, 
					      const std::string &st_name, 
					      int fc_hor ) {

    // Building filename... 
    std::string fname = this->pattern;
    OPAQ::StringTools::replaceAll( fname, POLLUTANT_PLACEHOLDER, pol_name );
    OPAQ::StringTools::replaceAll( fname, STATION_PLACEHOLDER, st_name );
    OPAQ::StringTools::replaceAll( fname, FCHOR_PLACEHOLDER, std::to_string(fc_hor) );
    OPAQ::StringTools::replaceAll( fname, MODEL_PLACEHOLDER, this->getName() );
    OPAQ::StringTools::replaceAll( fname, MCID_PLACEHOLDER,  this->mcid );
    
    return fname;
  }


  
  /* ============================================================================
     Construct sample for the OVL_IRCEL_model1 configuration
     ========================================================================== */
  int OVL_IRCEL_model1::makeSample( double *sample, OPAQ::Station *st, OPAQ::Pollutant *pol, 
				    const OPAQ::DateTime &baseTime, 
				    const OPAQ::DateTime &fcTime, 
				    const OPAQ::ForecastHorizon &fc_hor ) {
    
    int have_sample = 0; // return code, 0 for success

    // -----------------------
    // Getting data providers --> stored in main model...
    // -----------------------
    DataProvider *obs   = getInputProvider();
    DataProvider *meteo = getMeteoProvider();
    // AQNetwork    *net   = getAQNetworkProvider()->getAQNetwork();
    
    // -----------------------
    // Get the meteo input
    // -----------------------
    // we shift the meteo basetime to the forecast timebase, i.e. dayN ! 
    meteo->setBaseTime( fcTime );
        
    // BLH for dayN, offsets relative from fcTime (set by setBaseTime in the meteo provider)
    TimeInterval dayNbegin(0);
    TimeInterval dayNend(18*3600); // 1x meteto TimeResultion aftrekken : - getTimeResolution();
    std::vector<double> blh  = meteo->getValues( dayNbegin, dayNend, st->getMeteoId(), p_blh );
    
    // -----------------------------
    // get the morning concentration
    // -----------------------------
    TimeInterval begMor(0);
    TimeInterval endMor((this->mor_agg-1)*3600); // met mor_agg uur of eentje aftrekken ????
    std::vector<double> xx_morn = obs->getValues( begMor, endMor, pol->getName(), st->getName() );
    
    /*
    printPar( "morning concentrations" , xx_morn );
    */

    // -----------------------
    // build sample
    // -----------------------
    
    // add some more checks for missing values,  if sample not complete... either estimate value
    // from climatology ?? or don't run forecast...
    
    sample[0] = log(1 + mean_missing( xx_morn, obs->getNoData() ) );     // PMMOR
    sample[1] = log(1 + mean_missing(blh,meteo->getNoData( p_blh ) ) );  // BLH
    
    return have_sample;
  }
  
}

OPAQ_REGISTER_PLUGIN(OPAQ::OVL_IRCEL_model1);  
