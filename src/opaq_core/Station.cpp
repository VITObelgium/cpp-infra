#include "Station.h"

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
}
