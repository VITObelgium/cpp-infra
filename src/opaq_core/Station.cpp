#include "Station.h"

#include <algorithm>

namespace opaq
{

Station::Station(std::string name, std::string desc, std::string meteoId)
: _name(std::move(name))
, _desc(std::move(desc))
, _meteoId(std::move(meteoId))
{
}

Station::Station(long id, double x, double y, double z, std::string name, std::string desc, std::string meteoId)
: Point(id, x, y, z)
, _name(std::move(name))
, _desc(std::move(desc))
, _meteoId(std::move(meteoId))
{
}

std::ostream& operator<<(std::ostream& os, const Station& s)
{
    os << "[station " << s.getId() << "]: " << s._name
       << ", x=" << s.getX()
       << ", y=" << s.getY()
       << ", z=" << s.getZ()
       << ", meteo fc ID=" << s.getMeteoId();

    return os;
}

void Station::addPollutant(const Pollutant& p)
{
    _pollutants.push_back(p);
}

const std::string& Station::getName() const
{
    return _name;
}

const std::string& Station::getDescription() const
{
    return _desc;
}

const std::string& Station::getMeteoId() const
{
    return _meteoId;
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
           _meteoId == other._meteoId;
}
}
