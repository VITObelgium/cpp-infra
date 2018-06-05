/**
 *  XMLAQNetProvider.h
 *
 *  Created on: Feb 19, 2014
 *      Author: maiheub
 *
 */

#pragma once

#include "AQNetwork.h"
#include "AQNetworkProvider.h"

namespace opaq {

class XMLAQNetProvider : public AQNetworkProvider
{
public:
    static std::string name();

    // component members
    // throws OPAQ::BadConfigurationException
    void configure(const infra::ConfigNode& configuration, const std::string& componentName, IEngine& engine) override;

    // AQNetowrk functions
    virtual AQNetwork& getAQNetwork() override;

private:
    AQNetwork _net;
};

} /* namespace IRCEL */
