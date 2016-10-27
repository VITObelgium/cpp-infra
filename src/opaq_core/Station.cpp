#include "Station.h"

#include <algorithm>

namespace OPAQ
{

Station::Station(std::string name, std::string desc, std::string meteoId)
: _name(std::move(name))
, _desc(std::move(desc))
, _meteoId(std::move(meteoId))
{
}

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
