#ifndef __OVL_IRCEL_model1_H
#define __OVL_IRCEL_model1_H

#include <opaq.h>
#include "MLP_FeedForwardModel.h"

namespace OPAQ {

  class OVL_IRCEL_model1 : public OPAQ::MLP_FeedForwardModel {

  public:
    OVL_IRCEL_model1();
    ~OVL_IRCEL_model1();

    // OPAQ::Component methods
    virtual void configure (TiXmlElement * configuration) 
      throw (OPAQ::BadConfigurationException);

    virtual int getMissingValue( void ) { return missing_value; }

    virtual std::string getFFNetFile( const std::string &pol_name, 
				      const std::string &st_name, 
				      int fc_hor );

    virtual int makeSample( double *sample, OPAQ::Station *st, OPAQ::Pollutant *pol, 
			    const OPAQ::DateTime &baseTime, 
			    const OPAQ::DateTime &fcTime, 
			    const OPAQ::TimeInterval &fc_hor );

  private:
    std::string pattern;       //!< feed forward network file pattern
    std::string mcid;          //!< morning concentration id, 9UT, 17UT, 7CST etc..
    int         mor_agg;       //!< morning aggregation hour
    int         missing_value; //!< missing value, read from configuration

    std::string name;          //!< model name
  };
  
} // namespace
#endif /* #ifndef __OVL_IRCEL_model1_H */
