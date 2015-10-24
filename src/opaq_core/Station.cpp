#include "Station.h"

namespace OPAQ {

  Station::Station(){
  }

  Station::~Station() {
  }

  std::ostream& operator<<(std::ostream& os, const Station& s ) {

    os << "[station " << s.getId() << " ]: " << s.name 
       << ", x=" << s.getX()
       << ", y=" << s.getX()
       << ", z=" << s.getX()
       << ", meteo fc ID="<< s.getMeteoId() << std::endl;
      return os;
  }

}
