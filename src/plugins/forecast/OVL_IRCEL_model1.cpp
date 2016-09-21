#include <fstream>

#include <opaq.h>
#include "OVL_IRCEL_model1.h"

#define epsilon 1e-6
  
namespace OPAQ {

  OVL_IRCEL_model1::OVL_IRCEL_model1() :
	  p_t2m( "P01" ),         // t2m in IRCEL meteo provider
	  p_wsp10m( "P03" ),      // wind speed 10 m in IRCEL meteo provider
  	  p_wdir10m( "P04" ),     // wind direction 10 m
	  p_blh( "P07" ),         // boundary layer height
	  p_cc( "P13" ),          // low cloud cover
	  mor_agg( -1 )           // default is all
  {
	  sample_size = 2;
  }

  OVL_IRCEL_model1::~OVL_IRCEL_model1() {};

  /* ============================================================================
     Implementation of the configure method
     ========================================================================== */
  void OVL_IRCEL_model1::configure (TiXmlElement * cnf, IEngine&) {
    
    try {
      // read the path to the architecture files
      this->pattern = XmlTools::getText(cnf, "ffnetfile_pattern" );

    } catch (ElementNotFoundException & e) {
      throw BadConfigurationException(e.what());
    }
    
    // read morning aggregation (optional)
    try {
    	this->mor_agg = atoi(XmlTools::getText( cnf, "mor_agg" ).c_str() );
    } catch (const ElementNotFoundException&) {
    	this->mor_agg = 24; // take all available observations for the day
    }

    // read missing value for this model (optional)
    try {
    	this->missing_value = atoi(XmlTools::getText( cnf, "missing_value" ).c_str() );
    } catch ( ... ) { };



  }
  
  /* ============================================================================
     Construct sample for the OVL_IRCEL_model1 configuration
     ========================================================================== */
  int OVL_IRCEL_model1::makeSample( double *sample, const OPAQ::Station& st,
		  const OPAQ::Pollutant& pol, Aggregation::Type aggr,
		  const OPAQ::DateTime &baseTime, const OPAQ::DateTime &fcTime,
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
    t2 = fcTime + OPAQ::TimeInterval(24, TimeInterval::Hours ) - meteo->getTimeResolution();
    TimeSeries<double> blh  = meteo->getValues( t1, t2, st.getMeteoId(), p_blh );

    // -----------------------
    // build sample
    // -----------------------
    
    // 0. -------------------------------------------------------------------------------
    // sample[0] is the mean morning concentration of the measured pollutant we're trying to forecast
    t1 = DateTimeTools::floor( baseTime, DateTimeTools::FIELD_DAY );
    t2 = t1 + OPAQ::TimeInterval( this->mor_agg-1, TimeInterval::Hours ); // mor_agg uur of eentje aftrekken ?

    OPAQ::TimeSeries<double> xx_morn = obs->getValues(t1, t2, st.getName(), pol.getName() ); // no aggregation
    double xx = mean_missing( xx_morn.values(), obs->getNoData() );
    if ( fabs( xx - obs->getNoData() ) > epsilon ) sample[0] = log(1 + xx );
    else have_sample++;


    // 1. -------------------------------------------------------------------------------
    // sample[1] is the mean/min/max boundary layer height of the forecast day, depending on
    // the aggregation time & polluant.
    if ( aggr == OPAQ::Aggregation::Max1h ) {
    	if ( ! pol.getName().compare("o3") ) {
    		// take the maximum BLH for O3 forecasts
    		xx = max_missing( blh.values(), meteo->getNoData( p_blh ) );
    	} else {
    		// take the minimum BLH
    		xx = min_missing( blh.values(), meteo->getNoData( p_blh ) );
    	}
    } else {
    	// also for max8h we use the daily averages
    	xx = mean_missing( blh.values(), meteo->getNoData( p_blh ) );
    }
    if ( fabs( xx - meteo->getNoData(p_blh) ) > epsilon ) sample[1] = log(1 + xx );
    else {
    	have_sample++;
    	// TODO perhaps provide some kind of climatology to fill the missing sample...
    }
    
    return have_sample;
  }
  
}

OPAQ_REGISTER_PLUGIN(OPAQ::OVL_IRCEL_model1);  
