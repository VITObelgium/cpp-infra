#include <fstream>

#include <opaq.h>
#include "OVL_IRCEL_model1.h"

// a list of meteo variable names relevant for this model
// we should acutally better put this in the config file and allow the user
// to configure the ff model feature vector from the configuration
const std::string p_t2m     = "P01"; // 2m temperature
const std::string p_wsp10m  = "P03"; // wind speed 10 m
const std::string p_wdir10m = "P04"; // wind direction 10 m 
const std::string p_blh     = "P07"; // boundary layer height
const std::string p_cc      = "P12"; // total cloud cover
  
namespace OPAQ {

  OVL_IRCEL_model1::OVL_IRCEL_model1() {
    missing_value = -9999; // set default missing value, overwritting in xml config
    mor_agg       = -1;
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

      // read missing value for this model
      this->missing_value = atoi(XmlTools::getText( cnf, "missing_value" ).c_str() );

    } catch (ElementNotFoundException & e) {
      throw BadConfigurationException(e.what());
    }
    
    // read morning aggregation (optional)
    try {
    	this->mor_agg = atoi(XmlTools::getText( cnf, "mor_agg" ).c_str() );
    } catch (ElementNotFoundException & e) {
    	this->mor_agg = 24; // take all available observations for the day
    }

  }
  
  /* ============================================================================
     Construct sample for the OVL_IRCEL_model1 configuration
     ========================================================================== */
  int OVL_IRCEL_model1::makeSample( double *sample, const OPAQ::Station& st, const OPAQ::Pollutant& pol,
				    const OPAQ::DateTime &baseTime, const OPAQ::DateTime &fcTime,
				    const OPAQ::TimeInterval &fc_hor ) {

	OPAQ::DateTime t1, t2;
	int have_sample = 0; // return code, 0 for success

    // -----------------------
    // Getting data providers --> stored in main model...
    // -----------------------
    DataProvider *obs   = getInputProvider();
    MeteoProvider *meteo = getMeteoProvider();
    
    // -----------------------
    // Get the meteo input
    // -----------------------

    // BLH for dayN, offsets relative from fcTime (set by setBaseTime in the meteo provider)
    t1 = fcTime + OPAQ::TimeInterval(0);
    t2 = fcTime + OPAQ::TimeInterval(24, TimeInterval::Hours ) - meteo->getTimeResolution();
    TimeSeries<double> blh  = meteo->getValues( t1, t2, st.getMeteoId(), p_blh );

    // write some debugging output to a file ...
    /*
    std::ofstream fs;
    fs.open( std::string( "debug_" ) + st->getName() + "_" + pol->getName() + "_"
    		+ std::to_string( baseTime.getYear() ) + "-"
    		+ std::to_string( baseTime.getMonth() ) + "-"
			+ std::to_string( baseTime.getDay() )
    		+ ".txt" );
    fs << st->getMeteoId() << std::endl;
    fs << blh;
    fs.close();
     */
    
    // -----------------------------
    // get the morning concentration
    // -----------------------------
    t1 = DateTimeTools::floor( baseTime, DateTimeTools::FIELD_DAY );
    t2 = t1 + OPAQ::TimeInterval( this->mor_agg-1, TimeInterval::Hours ); // mor_agg uur of eentje aftrekken ?

    OPAQ::TimeSeries<double> xx_morn = obs->getValues(t1, t2, st.getName(), pol.getName() ); // no aggregation

    // -----------------------
    // build sample
    // -----------------------
    
    // add some more checks for missing values,  if sample not complete... either estimate value
    // from climatology ?? or don't run forecast...
    
    sample[0] = log(1 + mean_missing( xx_morn.values(), obs->getNoData() ) );      // PMMOR
    sample[1] = log(1 + mean_missing( blh.values(), meteo->getNoData( p_blh ) ) );  // BLH
    
    return have_sample;
  }
  
}

OPAQ_REGISTER_PLUGIN(OPAQ::OVL_IRCEL_model1);  
