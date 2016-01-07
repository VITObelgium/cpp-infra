/*
 * OVL.h
 *
 *  Created on: 2015
 *      Author: bino.maiheu@vito.be
 *
 *
 *  The OVL model, which makes use of the individual different FF models, but adds real time corrections &
 *  model selection
 */

#ifndef OVL_H_
#define OVL_H_

#include <string>
#include <map>
#include <tuple>
#include <opaq.h>

namespace OPAQ {
  
class OVL: virtual public OPAQ::Model {
public:
	OVL();
    virtual ~OVL();

    // OPAQ::Component methods
    virtual void configure (TiXmlElement * configuration)
      throw (OPAQ::BadConfigurationException);

    // the configure method should also be implemented in the derived class...
    // OPAQ::Model methods --> run for this particular fcTime...
    virtual void run();
    

    // define a nested struct to hold the configuration of
    // a station
    struct StationConfig {
    	int         rtc_mode;
    	int         rtc_param;
    	std::string model_name;
    };


private:
    LOGGER_DEC();

    std::string tune_mode;       //! the selected tune mode for how OVL was optimized.
    int         missing_value;   //! a missing value
    bool        output_raw;      //! store the raw output (if not all models are present in OPAQ

    OPAQ::TimeInterval hindcast;

    void _parseTunes( TiXmlElement *lst );

    /**
     * this one is a map for
     * pollutant -> aggregation, followed by a list of OVL::StationConfig structures
     * using std::tuples for this, a key pair can be made via
     * std::make_tuple( "pm10", "dayavg", "40ML01", fc_hor )
     * where fc_hor is the forecast horizon in days...
     * we are not using TimeInterval in the tuple (could do) because we would need to
     * add some stuff in the implementation of the TimeInterval to be able to use it in a tuple
     */
    std::map<std::tuple<std::string, OPAQ::Aggregation::Type, std::string, int>, OVL::StationConfig> _conf;
  };
  
  
} /* namespace OPAQ */
#endif /* OVL_H_ */
