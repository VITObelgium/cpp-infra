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

#pragma once

#include "ComponentManager.h"
#include "Model.h"

#include <map>
#include <string>
#include <tuple>

namespace opaq {

class OVL : public Model
{
public:
    OVL();

    static std::string name();

    // OPAQ::Component methods
    // throws OPAQ::BadConfigurationException
    void configure(const infra::ConfigNode& configuration, const std::string& componentName, IEngine& engine) override;

    // the configure method should also be implemented in the derived class...
    // OPAQ::Model methods --> run for this particular fcTime...
    virtual void run() override;

    // define a nested struct to hold the configuration of
    // a station
    struct StationConfig
    {
        int rtc_mode;
        int rtc_param;
        std::string model_name;
    };

private:
    void parseTuneList(const infra::ConfigNode& list);
    void parseTuneElement(const infra::ConfigNode& tuneEl);

    ComponentManager* _componentMgr;

    std::string _tune_mode; //! the selected tune mode for how OVL was optimized.
    bool _output_raw;       //! store the raw output (if not all models are present in OPAQ
    bool _debug_output;     //! activate debugging output

    chrono::days _hindcast;

    /**
     * this one is a map for
     * pollutant -> aggregation, followed by a list of OVL::StationConfig structures
     * using std::tuples for this, a key pair can be made via
     * std::make_tuple( "pm10", "dayavg", "rmse", "40ML01", fc_hor )
     * where fc_hor is the forecast horizon in days...
     * we are not using TimeInterval in the tuple (could do) because we would need to
     * add some stuff in the implementation of the TimeInterval to be able to use it in a tuple
     */
    std::map<std::tuple<std::string, Aggregation::Type, std::string, std::string, int>, OVL::StationConfig> _conf;
};

} /* namespace OPAQ */
