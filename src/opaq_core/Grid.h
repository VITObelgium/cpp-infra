/*
 * Grid.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys, maiheub
 */

#ifndef OPAQ_GRID_H_
#define OPAQ_GRID_H_

#include <string>
#include <vector>

#include "Cell.h"

namespace OPAQ {

  /**
     A class to contain the interpolation grid.
     Basically is a container class for a list of gridcells, which can be fetched as a 
     vector by the getCells member function. 
   */
  class Grid {
  public:
    Grid();
    virtual ~Grid();
    
    /** Get the gridcells
	Returns a reference to the vector of cells (pointers) */
    std::vector<Cell*> & getCells() { return cells; }
    //std::vector<Pollutant*> & getPollutants() { return pollutants; }
    
  private:
    std::vector<Cell*> cells;
    //std::vector<Pollutant*> pollutants;
  };
  
} /* namespace opaq */
#endif /* OPAQ_GRID_H_ */
