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

namespace opaq
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
    bool containsStation(const std::string& stationCode) const noexcept;

    void addStation(Station station);

    /** 
    Return a reference to the list of stations in the network
    */
    const std::vector<Station>& getStations() const { return _stations; }

private:
    std::vector<Station> _stations;
};
}
