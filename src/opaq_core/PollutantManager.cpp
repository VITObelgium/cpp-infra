#include "PollutantManager.h"

namespace OPAQ
{

namespace Config
{

PollutantManager::PollutantManager()
: _logger("OPAQ::Config::PollutantManager")
{
}

OPAQ::Pollutant* PollutantManager::find(const std::string& name)
{
    auto iter = std::find_if(_pollutants.begin(), _pollutants.end(), [&name] (auto& pollutant) {
        return pollutant.getName() == name;
    });

    if (iter == _pollutants.end())
    {
        _logger->warn("Pollutant with name '{}' not found.", name);
        return nullptr;
    }

    return &(*iter);
}

void PollutantManager::configure(TiXmlElement const* config)
{
    _pollutants.clear();
    const TiXmlElement* pollutantElement = config->FirstChildElement("pollutant");
    while (pollutantElement)
    {
        OPAQ::Pollutant pol(pollutantElement);
        _pollutants.push_back(pol);
        pollutantElement = pollutantElement->NextSiblingElement("pollutant");
    }
}

std::ostream& operator<<(std::ostream& os, const PollutantManager& s)
{
    for (auto& pol : s._pollutants)
    {
        os << pol << std::endl;
    }
    return os;
}
}
}
