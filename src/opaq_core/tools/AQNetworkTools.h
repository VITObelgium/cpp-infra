/*
 * AQNetworkTools.h
 *
 *  Created on: Feb 27, 2014
 *      Author: vlooys
 */

#pragma once

#include "../AQNetwork.h"
#include "../Station.h"
#include <algorithm>
#include <vector>

namespace OPAQ
{

namespace AQNetworkTools
{
/**
    Checks whether the station contains a pollutant
    \param station a pointer to the station
    \param pollutant a reference to the pollutant
    \return true or false
 */
bool stationHasPollutant(Station* station, Pollutant& pollutant);

}
}

