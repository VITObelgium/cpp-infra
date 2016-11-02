#include "PollutantManager.h"

#include "Exceptions.h"

namespace opaq
{
namespace Config
{

PollutantManager::PollutantManager()
: _logger("OPAQ::Config::PollutantManager")
{
}

Pollutant PollutantManager::find(const std::string& name)
{
    auto iter = std::find_if(_pollutants.begin(), _pollutants.end(), [&name] (auto& pollutant) {
        return pollutant.getName() == name;
    });

    if (iter == _pollutants.end())
    {
        throw InvalidArgumentsException("No pollutant with name: {}", name);
    }

    return *iter;
}

void PollutantManager::configure(TiXmlElement const* config)
{
    _pollutants.clear();
    const TiXmlElement* pollutantElement = config->FirstChildElement("pollutant");
    while (pollutantElement)
    {
        Pollutant pol(pollutantElement);
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
