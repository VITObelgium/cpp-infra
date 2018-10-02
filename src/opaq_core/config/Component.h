#pragma once

#include "Plugin.h"
#include "infra/xmldocument.h"

#include <string>

class TiXmlElement;

namespace opaq {
namespace config {

/**
 * Component configuration class
 * Not to be confused with the OPAQ::Component class, which contains the actual
 * component functionality. This is a configuration wrapper, which contains the 
 * XML configuration for the OPAQ Component. 
 */
struct Component
{
    std::string name;
    Plugin plugin;
    inf::XmlNode config;
};
}
}
