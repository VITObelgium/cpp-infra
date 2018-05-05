#include <stdlib.h>
#include <string>

#include "Pollutant.h"
#include "infra/configdocument.h"

#include <sstream>

namespace opaq {

using namespace infra;

Pollutant::Pollutant()
: _id(0)
{
}

Pollutant::Pollutant(long id, std::string name, std::string unit, std::string desc)
: _id(id)
, _name(std::move(name))
, _unit(std::move(unit))
, _desc(std::move(desc))
{
}

Pollutant::Pollutant(const ConfigNode& el)
{
    _id   = el.attribute<int64_t>("id").value();
    _name = std::string(el.attribute("name"));
    _unit = std::string(el.attribute("unit"));

    _desc = el.value();
}

std::string Pollutant::toString() const
{
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Pollutant& s)
{
    os << "[pollutant]: " << s._name << " : " << s._desc;
    return os;
}
}
