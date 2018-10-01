#include "PollutantManager.h"

#include "Exceptions.h"
#include "infra/configdocument.h"

#include <algorithm>

namespace opaq {
namespace config {

using namespace inf;

Pollutant PollutantManager::find(const std::string& name)
{
    auto iter = std::find_if(_pollutants.begin(), _pollutants.end(), [&name](auto& pollutant) {
        return pollutant.getName() == name;
    });

    if (iter == _pollutants.end()) {
        throw InvalidArgument("No pollutant with name: {}", name);
    }

    return *iter;
}

void PollutantManager::configure(const ConfigNode& config)
{
    _pollutants.clear();
    for (auto& pollutantElement : config.children("pollutant")) {
        _pollutants.emplace_back(pollutantElement);
    }
}

std::ostream& operator<<(std::ostream& os, const PollutantManager& s)
{
    for (auto& pol : s._pollutants) {
        os << pol << std::endl;
    }
    return os;
}
}
}
