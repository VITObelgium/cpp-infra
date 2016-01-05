#ifndef __OVL_IRCEL_model2_H
#define __OVL_IRCEL_model2_H

#include <opaq.h>
#include "MLP_FeedForwardModel.h"

namespace OPAQ {

  class OVL_IRCEL_model2 : public OPAQ::MLP_FeedForwardModel {

  public:
    OVL_IRCEL_model2();
    ~OVL_IRCEL_model2();

    // OPAQ::Component methods
    virtual void configure (TiXmlElement * configuration) 
      throw (OPAQ::BadConfigurationException);

    virtual int getMissingValue( void ) { return missing_value; }

    virtual int makeSample( double *sample, const OPAQ::Station& st, const OPAQ::Pollutant& pol,
			    const OPAQ::DateTime &baseTime, const OPAQ::DateTime &fcTime,
			    const OPAQ::TimeInterval &fc_hor );

  private:
    int         mor_agg;       //!< morning aggregation hour
    int         missing_value; //!< missing value, read from configuration
  };
  
} // namespace
#endif /* #ifndef __OVL_IRCEL_model2_H */
