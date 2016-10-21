/**
 *  XMLAQNetProvider.h
 *
 *  Created on: Feb 19, 2014
 *      Author: maiheub
 *
 */

#pragma once

#include <opaq.h>

namespace OPAQ
{

class XMLAQNetProvider : public OPAQ::AQNetworkProvider
{
public:
    XMLAQNetProvider();

    // component members
    // throws OPAQ::BadConfigurationException
    void configure(TiXmlElement* configuration, const std::string& componentName, IEngine& engine) override;

    // AQNetowrk functions
    virtual OPAQ::AQNetwork& getAQNetwork() override;

private:
    OPAQ::AQNetwork _net;
    Logger _logger;
};

} /* namespace IRCEL */
