#pragma once

#include "Logger.h"
#include "data/IStationInfoProvider.h"

#include <map>

namespace opaq
{

class StationInfoProvider : public IStationInfoProvider
{
public:
    StationInfoProvider();

    static std::string name();

    void configure(TiXmlElement* configuration, const std::string& componentName, IEngine& engine) override;

    std::vector<Station> getStations(Pollutant pollutant) override;

private:
    void readFile(Pollutant pollutant);

    Logger _logger;
    std::string _pattern;
    bool _configured;
    std::map<std::string, std::vector<Station>> _stations; // key: pollutant name
    std::string _gisType;
};

}
