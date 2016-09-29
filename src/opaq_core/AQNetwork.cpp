#include "AQNetwork.h"

#include <algorithm>

namespace OPAQ
{

Station* AQNetwork::findStation(const std::string& name)
{
    auto iter = std::find_if(_stations.begin(), _stations.end(), [&name](Station* station) {
        return station->getName() == name;
    });

    return iter == _stations.end() ? nullptr : *iter;
}

}
