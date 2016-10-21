#include "Station.h"

#include <algorithm>

namespace OPAQ
{

std::ostream& operator<<(std::ostream& os, const Station& s)
{
    os << "[station " << s.getId() << " ]: " << s._name
       << ", x=" << s.getX()
       << ", y=" << s.getX()
       << ", z=" << s.getX()
       << ", meteo fc ID=" << s.getMeteoId() << std::endl;
    return os;
}

bool Station::measuresPollutant(const Pollutant& pol) const noexcept
{
    auto iter = std::find_if(_pollutants.begin(), _pollutants.end(), [&pol](const Pollutant& polIter) {
        return polIter.getName() == pol.getName();
    });

    return iter != _pollutants.end();
}

}
