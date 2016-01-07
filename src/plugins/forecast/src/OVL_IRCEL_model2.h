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

    virtual int makeSample( double *sample, const OPAQ::Station& st, const OPAQ::Pollutant& pol,
        		    OPAQ::Aggregation::Type aggr, const OPAQ::DateTime &baseTime,
    				const OPAQ::DateTime &fcTime, const OPAQ::TimeInterval &fc_hor );

  private:
    const std::string p_t2m;     // t2m
    const std::string p_wsp10m;  // wind speed 10 m
    const std::string p_wdir10m; // wind direction 10 m
    const std::string p_blh;     // boundary layer height
    const std::string p_cc;      // tot

    int mor_agg;                 //!< morning aggregation hour
  };
  
} // namespace
#endif /* #ifndef __OVL_IRCEL_model2_H */
