#include "PollutantManager.h"

namespace OPAQ
{

namespace Config
{

PollutantManager::PollutantManager()
: logger("OPAQ::Config::PollutantManager")
{
}

OPAQ::Pollutant* PollutantManager::find(const std::string& name)
{
    auto iter = std::find_if(pollutants.begin(), pollutants.end(), [&name] (auto& pollutant) {
        return pollutant.getName() == name;
    });

    if (iter == pollutants.end())
    {
        logger->warn("Pollutant with name '{}' not found.", name);
        return nullptr;
    }

    return &(*iter);
}

void PollutantManager::configure(TiXmlElement const* config)
{
    pollutants.clear();
    const TiXmlElement* pollutantElement = config->FirstChildElement("pollutant");
    while (pollutantElement)
    {
        OPAQ::Pollutant pol(pollutantElement);
        pollutants.push_back(pol);
        pollutantElement = pollutantElement->NextSiblingElement("pollutant");
    }
}

std::ostream& operator<<(std::ostream& os, const PollutantManager& s)
{
    for (auto& pol : s.pollutants)
    {
        os << pol << std::endl;
    }
    return os;
}
}
}
