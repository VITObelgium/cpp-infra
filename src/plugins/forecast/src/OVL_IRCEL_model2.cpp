#include <fstream>

#include <opaq.h>
#include "OVL_IRCEL_model2.h"

#define epsilon 1e-6

// a list of meteo variable names relevant for this model
// we should acutally better put this in the config file and allow the user
// to configure the ff model feature vector from the configuration
const std::string p_t2m     = "P01"; // 2m temperature
const std::string p_wsp10m  = "P03"; // wind speed 10 m
const std::string p_wdir10m = "P04"; // wind direction 10 m 
const std::string p_blh     = "P07"; // boundary layer height
const std::string p_cc      = "P12"; // total cloud cover

namespace OPAQ {

  OVL_IRCEL_model2::OVL_IRCEL_model2() {
    missing_value = -9999; // set default missing value, overwritting in xml config
    mor_agg       = -1;
    sample_size   = 7;
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
     Construct sample for the OVL_IRCEL_model2 configuration
     ========================================================================== */
  int OVL_IRCEL_model2::makeSample( double *sample, const OPAQ::Station& st,
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
    t2 = fcTime + OPAQ::TimeInterval(24*3600) - meteo->getTimeResolution();

    TimeSeries<double> blh  = meteo->getValues( t1, t2, st.getMeteoId(), p_blh );
    TimeSeries<double> cc   = meteo->getValues( t1, t2, st.getMeteoId(), p_cc );
    
    t1 = fcTime - TimeInterval(12*3600); // dayN-1 : 12:00 to end, including 12:00
    t2 = fcTime + TimeInterval(12*3600) - meteo->getTimeResolution(); // day N : 00:00 to 12:00 (excluding)
    
    TimeSeries<double> wsp  = meteo->getValues( t1, t2, st.getMeteoId(), p_wsp10m );
    TimeSeries<double> wdir = meteo->getValues( t1, t2, st.getMeteoId(), p_wdir10m );
    
    // calculate the wind speed u and v components from the wsp and wdir
    std::vector<double> u,v;
    u.resize(wsp.size());
    v.resize(wsp.size());
    for ( unsigned int ii=0; ii< u.size(); ii++ ) {
    	if ( ( wdir.value(ii) == meteo->getNoData( p_wdir10m ) ) ||
    		 ( wsp.value(ii)  == meteo->getNoData( p_wsp10m ) ) ) {
    		u[ii] = meteo->getNoData( p_wsp10m );
    		v[ii] = meteo->getNoData( p_wsp10m );
      } else {
    	  u[ii] = wsp.value(ii) * cos( OPAQ::Math::Pi * wdir.value(ii) / 180. );
    	  v[ii] = wsp.value(ii) * sin( OPAQ::Math::Pi * wdir.value(ii) / 180. );
      }
    }


#ifdef DEBUG
    // some debugging information to a file...
    std::ofstream fs;
    fs.open( std::string( "debug_" ) + st.getName() + "_" + pol.getName() + "_"
    		+ std::to_string( baseTime.getYear() ) + "-"
    		+ std::to_string( baseTime.getMonth() ) + "-"
			+ std::to_string( baseTime.getDay() )
    		+ ".txt" );
    fs << st.getMeteoId() << std::endl;
    fs << "BLH : " << std::endl;
    fs << blh;
    fs << "CC : " << std::endl;
    fs << cc;
    fs << "WSP : " << std::endl;
    fs << wsp;
    fs << "WDIR : " << std::endl;
    fs << wdir;
    fs.close();
#endif


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
    // sample[1] is the mean concentration of the measured pollutant of day-1
    t1 = DateTimeTools::floor( baseTime, DateTimeTools::FIELD_DAY ) - TimeInterval( 1, TimeInterval::Days );
    t2 = DateTimeTools::floor( baseTime, DateTimeTools::FIELD_DAY ) - obs->getTimeResolution();  // 1x meteto TimeResultion aftrekken : - getTimeResolution();

    OPAQ::TimeSeries<double> xx_yest = obs->getValues( t1, t2, st.getName(), pol.getName() );
    xx = mean_missing( xx_morn.values(), obs->getNoData() );
    if ( fabs( xx - obs->getNoData() ) > epsilon ) sample[1] = log(1 + xx );
    else have_sample++;

    // 2. -------------------------------------------------------------------------------
    // sample[2] is the mean/min/max boundary layer height of the forecast day, depending on
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
    if ( fabs( xx - meteo->getNoData(p_blh) ) > epsilon ) sample[2] = log(1 + xx );
    else {
    	have_sample++;
    	// TODO perhaps provide some kind of climatology to fill the missing sample...
    }

    // 3. -------------------------------------------------------------------------------
    // sample[3] is the mean medium cloud cover for dayN
    xx = mean_missing( cc.values(), meteo->getNoData( p_cc ) );
    if ( fabs( xx - meteo->getNoData(p_cc) ) > epsilon ) sample[3] = log(1 + xx );
    else have_sample++;
    
    // 4. and 5. ------------------------------------------------------------------------
    // sample[4] and sample[5] are zoneal & meridional wind vectors, note that above
    // we have used the nodata value of the wsp10m
    sample[4] = mean_missing( u, meteo->getNoData( p_wsp10m ) ); // wdir x
    if ( fabs( sample[4] - meteo->getNoData(p_wsp10m) ) < epsilon ) have_sample++;

    sample[5] = mean_missing( v, meteo->getNoData( p_wsp10m ) ); // wdir y
    if ( fabs( sample[5] - meteo->getNoData(p_wsp10m) ) < epsilon ) have_sample++;

    // 6. -------------------------------------------------------------------------------
    // get the weekday : week/weekend ?
    int weekend = 0;
    if ( ( fcTime.getDayOfWeek() == 0 ) || ( fcTime.getDayOfWeek() == 6 ) ) weekend = 1;
    sample[6] = weekend; // weekend


    return have_sample;
  }
  
}

OPAQ_REGISTER_PLUGIN(OPAQ::OVL_IRCEL_model2);
