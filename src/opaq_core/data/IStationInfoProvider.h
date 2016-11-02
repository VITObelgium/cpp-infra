#pragma once

#include "Station.h"
#include "../Pollutant.h"
#include "../Component.h"

#include <vector>

namespace OPAQ
{

class IStationInfoProvider : public Component
{
public:
    virtual std::vector<Station> getStations(Pollutant pol) = 0;
};

}
