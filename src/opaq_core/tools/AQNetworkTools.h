/*
 * AQNetworkTools.h
 *
 *  Created on: Feb 27, 2014
 *      Author: vlooys
 */

#ifndef AQNETWORKTOOLS_H_
#define AQNETWORKTOOLS_H_

#include "../AQNetwork.h"
#include "../Station.h"
#include <vector>

namespace OPAQ {

  /** 
      Class containing a number of static air quality network tools
      In this class a number of static member functions are implemented which
      contain some functionality for querying an air quality network, for
      example checking whether a network contains a certain station or whether
      a station contains a certain pollutant
  */
  class AQNetworkTools {
  public:
    AQNetworkTools();
    virtual ~AQNetworkTools();
    
    /**
       Checks whether the air quality network contains a certain station
       \param network a reference to the air quality network
       \param stationCode a string reference indicating the station
       \return true or false
    */
    static bool containsStation (OPAQ::AQNetwork & network, std::string & stationCode) {
      std::vector<OPAQ::Station *> * stations = &network.getStations();
      std::vector<OPAQ::Station *>::iterator it = stations->begin();
      while (it != stations->end()) {
	OPAQ::Station * station = *it;
	if (station->getName().compare(stationCode) == 0) return true;
	it++;
      }
      return false;
    }


  /**
       Checks whether the station contains a pollutant
       \param station a pointer to the station
       \param pollutant a reference to the pollutant
       \return true or false
    */
    static bool stationHasPollutant (Station * station, Pollutant & pollutant) {
      std::vector<Pollutant *> pols = station->getPollutants();
      std::vector<Pollutant *>::iterator polIt = pols.begin();
      while (polIt != pols.end()) {
	Pollutant * pol = *polIt++;
	if (pol->getId() == pollutant.getId()) return true;
      }
      return false;
    }
  };
  
} 
#endif /* AQNETWORKTOOLS_H_ */
