#include "AQNetwork.h"

#include <algorithm>

namespace OPAQ
{

bool AQNetwork::containsStation(const std::string& stationCode) const noexcept
{
    auto iter = std::find_if(_stations.begin(), _stations.end(), [&stationCode](const std::unique_ptr<Station>& station) {
        return station->getName() == stationCode;
    });

    return iter != _stations.end();
}

void AQNetwork::addStation(std::unique_ptr<Station> station)
{
    _stations.emplace_back(std::move(station));
}

Station* AQNetwork::findStation(const std::string& name) const noexcept
{
    auto iter = std::find_if(_stations.begin(), _stations.end(), [&name](const std::unique_ptr<Station>& station) {
        return station->getName() == name;
    });

    return iter == _stations.end() ? nullptr : iter->get();
}

}
