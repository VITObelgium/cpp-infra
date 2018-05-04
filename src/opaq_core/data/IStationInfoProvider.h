#pragma once

#include "../Station.h"
#include "../Pollutant.h"
#include "../Component.h"

#include <vector>

namespace opaq
{

class IStationInfoProvider : public Component
{
public:
    virtual std::vector<Station> getStations(Pollutant pol, const std::string& gisType) = 0;
};

}
