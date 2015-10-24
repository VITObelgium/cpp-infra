#include "Cell.h"

namespace OPAQ {

  Cell::Cell(){
  }

  Cell::Cell( long ID, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax )
    : id(ID),xmin(xmin),xmax(xmax),ymin(ymin),ymax(ymax),zmin(zmin),zmax(zmax){
  }

  Cell::Cell( long ID, double xmin, double xmax, double ymin, double ymax )
    : id(ID),xmin(xmin),xmax(xmax),ymin(ymin),ymax(ymax),zmin(0.),zmax(0.){
  }
    
  Cell::~Cell() {
  }
  
}
