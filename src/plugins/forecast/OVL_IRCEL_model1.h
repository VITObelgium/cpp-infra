#ifndef __OVL_IRCEL_model1_H
#define __OVL_IRCEL_model1_H

#include "MLP_FeedForwardModel.h"

namespace opaq {

class OVL_IRCEL_model1 : public MLP_FeedForwardModel
{
public:
    OVL_IRCEL_model1();

    static std::string name();

    // OPAQ::Component methods
    // throws OPAQ::BadConfigurationException
    void configure(const inf::ConfigNode& configuration, const std::string& componentName, IEngine& engine) override;

    virtual int makeSample(double* sample, const Station& st, const Pollutant& pol,
        Aggregation::Type aggr, const chrono::date_time& baseTime,
        const chrono::date_time& fcTime, chrono::days fc_hor) override;

private:
    const std::string p_t2m;     // t2m
    const std::string p_wsp10m;  // wind speed 10 m
    const std::string p_wdir10m; // wind direction 10 m
    const std::string p_blh;     // boundary layer height
    const std::string p_cc;      // tot

    int mor_agg; //!< morning aggregation hour
};

} // namespace
#endif /* #ifndef __OVL_IRCEL_model1_H */
