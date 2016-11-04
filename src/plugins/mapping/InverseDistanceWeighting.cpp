
#include "InverseDistanceWeighting.h"
#include "PluginRegistration.h"

#include <tinyxml.h>

namespace opaq
{

using namespace std::chrono_literals;

InverseDistanceWeighting::InverseDistanceWeighting()
{
}

std::string InverseDistanceWeighting::name()
{
    return "idwmodel";
}

void InverseDistanceWeighting::configure(TiXmlElement* cnf, const std::string& componentName, IEngine&)
{
    setName(componentName);
}

void InverseDistanceWeighting::run()
{

}

OPAQ_REGISTER_STATIC_PLUGIN(InverseDistanceWeighting)

}

