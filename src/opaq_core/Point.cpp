#include "Point.h"

namespace OPAQ {

  Point::Point(){
  };

  Point::Point( long ID, double X, double Y ) 
    : id(ID),x(X),y(Y),z(0.) { 
  };

  Point::Point( long ID, double X, double Y, double Z ) 
    : id(ID),x(X),y(Y),z(Z) { 
  };

  Point::~Point(){
  };

}
