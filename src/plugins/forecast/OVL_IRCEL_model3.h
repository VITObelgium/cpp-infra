#ifndef __OVL_IRCEL_model3_H
#define __OVL_IRCEL_model3_H

#include "MLP_FeedForwardModel.h"

namespace opaq {

class OVL_IRCEL_model3 : public MLP_FeedForwardModel
{
public:
    OVL_IRCEL_model3();

    static std::string name();

    // OPAQ::Component methods
    // throws OPAQ::BadConfigurationException
    void configure(const infra::ConfigNode& configuration, const std::string& componentName, IEngine& engine) override;

    int makeSample(double* sample, const Station& st, const Pollutant& pol,
        Aggregation::Type aggr, const chrono::date_time& baseTime,
        const chrono::date_time& fcTime, chrono::days fc_hor) override;

private:
    const std::string p_t2m;     // 2m temperature
    const std::string p_wsp10m;  // wind speed 10 m
    const std::string p_wdir10m; // wind direction 10 m
    const std::string p_blh;     // boundary layer height
    const std::string p_cc;      // low cloud cover
    const std::string p_rh;      // relative humidity
    const std::string p_S;       // buyltinck-malet S parameter
    const std::string p_Transp;  // horizontal transport in BL (BLH x mean wind in BLH)

    int mor_agg; //!< morning aggregation hour
};

} // namespace
#endif /* #ifndef __OVL_IRCEL_model3_H */
