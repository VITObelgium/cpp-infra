#include "AQNetwork.h"

#include <algorithm>

namespace opaq
{

bool AQNetwork::containsStation(const std::string& stationCode) const noexcept
{
    auto iter = std::find_if(_stations.begin(), _stations.end(), [&stationCode](const Station& station) {
        return station.getName() == stationCode;
    });

    return iter != _stations.end();
}

void AQNetwork::addStation(Station station)
{
    _stations.emplace_back(std::move(station));
}

}
