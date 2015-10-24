/*
 * AQNetwork.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys, maiheub
 */

#ifndef OPAQ_AQNETWORK_H_
#define OPAQ_AQNETWORK_H_

#include <string>
#include <vector>

#include "Station.h"

namespace OPAQ {

  /**
     Representation of an Air Quality network
     Essentially nothing more than a list of stations
  */
  class AQNetwork {
  public:
    AQNetwork();
    virtual ~AQNetwork();
    
    /** 
	Return a reference to the list of stations in the network
    */
    std::vector<Station*> & getStations() {  return stations; }

    /**
       Finds the requested station in the network and returns a pointer to it
       The routine compares the given string and the station name
    */
    Station * findStation( std::string name );
    
  private:
    std::vector<Station *> stations;
  };
  
}
#endif /* #ifndef OPAQ_AQNETWORK_H_ */
