#include "Station.h"

#include <algorithm>

namespace opaq
{

Station::Station(std::string name, std::string desc, std::string meteoId)
: Station(std::move(name), std::move(desc), std::move(meteoId), 0.0)
{
}

Station::Station(std::string name, std::string desc, std::string meteoId, double beta)
: _name(std::move(name))
, _desc(std::move(desc))
, _meteoId(std::move(meteoId))
, _beta(beta)
{
}

Station::Station(long id, double x, double y, double z, std::string name, std::string desc, std::string meteoId, double beta)
: Point(id, x, y, z)
, _name(std::move(name))
, _desc(std::move(desc))
, _meteoId(std::move(meteoId))
, _beta(beta)
{
}

std::ostream& operator<<(std::ostream& os, const Station& s)
{
    os << "[station " << s.getId() << "]: " << s._name
       << ", x=" << s.getX()
       << ", y=" << s.getY()
       << ", z=" << s.getZ()
       << ", meteo fc ID=" << s.getMeteoId();

    if (s._beta != 0.0)
    {
        os << ", beta=" << s._beta;
    }

    return os;
}

bool Station::measuresPollutant(const Pollutant& pol) const noexcept
{
    auto iter = std::find_if(_pollutants.begin(), _pollutants.end(), [&pol](const Pollutant& polIter) {
        return polIter.getName() == pol.getName();
    });

    return iter != _pollutants.end();
}

bool Station::operator==(const Station& other) const noexcept
{
    return getId() == other.getId() &&
           getX() == other.getX() &&
           getY() == other.getY() &&
           getZ() == other.getZ() &&
           _name == other._name &&
           _desc == other._desc &&
           _meteoId == other._meteoId &&
           _beta == other._beta;
}

}
