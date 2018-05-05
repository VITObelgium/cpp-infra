#pragma once

#include "Logger.h"
#include "data/IStationInfoProvider.h"

#include <map>

namespace opaq {

class StationInfoProvider : public IStationInfoProvider
{
public:
    StationInfoProvider();

    static std::string name();

    void configure(const infra::ConfigNode& configuration, const std::string& componentName, IEngine& engine) override;

    std::vector<Station> getStations(Pollutant pollutant, const std::string& gisType) override;

private:
    void readFile(Pollutant pollutant, const std::string& gisType);

    Logger _logger;
    std::string _pattern;
    bool _configured;
    std::map<std::string, std::map<std::string, std::vector<Station>>> _stations; // key: pollutant name, gis type
};
}
