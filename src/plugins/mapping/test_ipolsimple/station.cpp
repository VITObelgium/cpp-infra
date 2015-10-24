#include <math.h>

#include "station.h"

station::station( std::string name, double x, double y, int type ) : 
  _name(name),_x(x),_y(y),_type(type) {
  _isActive = true;
  
  _value = pow(x/1000.,2)+pow(y/1000,2);
}

station::~station(){
}

std::ostream& operator<<(std::ostream &os, const station& s ) {
  //os << s._name << ", x=" << s._x << ", y=" << s._y << ", type=" << s._type << std::endl;
  os << "<station name=\"" << s._name 
     << "\" x=\"" << s._x 
     << "\" y=\"" << s._y 
     << "\" type=\""<< s._type << "\"/>";
  return os;
}
