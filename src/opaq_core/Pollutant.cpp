#include <stdlib.h>
#include <string>

#include "Pollutant.h"

namespace OPAQ {

  Pollutant::Pollutant(){
  }
   
  Pollutant::Pollutant( TiXmlElement const *el ) {
    //TODO : error handling to check whether the element is well formed

    // convert id_string to long (no direct long query in tinyxml)
    std::string str;
    el->QueryStringAttribute( "id", &str );
    this->id = atol( str.c_str() );

    el->QueryStringAttribute( "name", &(this->name) );
    el->QueryStringAttribute( "unit", &(this->unit) );
    this->desc = el->GetText();
  }
  
  Pollutant::~Pollutant(){
  }

  std::string Pollutant::toString() {
	  std::stringstream ss;
	  ss << *this;
	  return ss.str();
  }

  std::ostream& operator<<(std::ostream& os, const Pollutant& s ) {
    os << "[pollutant]: " << s.name << " : " << s.desc;
    return os;
  }

}
