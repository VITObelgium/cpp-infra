#include <stdlib.h>
#include <string>

#include "Pollutant.h"

namespace opaq
{

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

Pollutant::Pollutant(TiXmlElement const* el)
{

    // convert id_string to long (no direct long query in tinyxml)
    std::string str;
    el->QueryStringAttribute("id", &str);
    _id = atol(str.c_str());

    el->QueryStringAttribute("name", &(_name));
    el->QueryStringAttribute("unit", &(_unit));
    _desc = el->GetText();
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
