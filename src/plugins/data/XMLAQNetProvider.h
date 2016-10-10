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
    virtual void configure(TiXmlElement* configuration, IEngine& engine);

    // AQNetowrk functions
    virtual OPAQ::AQNetwork* getAQNetwork();

private:
    OPAQ::AQNetwork _net;
    Logger _logger;
};

} /* namespace IRCEL */
