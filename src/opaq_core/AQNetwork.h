/*
 * AQNetwork.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys, maiheub
 */

#pragma once

#include "Station.h"

#include <string>
#include <vector>
#include <memory>

namespace OPAQ
{

/**
     Representation of an Air Quality network
     Essentially nothing more than a list of stations
  */
class AQNetwork
{
public:
    /**
    Checks whether the air quality network contains a certain station
    \param stationCode a string reference indicating the station
    \return true or false
    */
    bool containsStation(const std::string& stationCode) const;

    void addStation(std::unique_ptr<Station> station);

    /** 
    Return a reference to the list of stations in the network
    */
    const std::vector<std::unique_ptr<Station>>& getStations() const { return _stations; }

    /**
       Finds the requested station in the network and returns a pointer to it
       The routine compares the given string and the station name
    */
    Station* findStation(const std::string& name);

private:
    std::vector<std::unique_ptr<Station>> _stations;
};
}
