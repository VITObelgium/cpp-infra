/*
 * AQNetworkTools.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: vlooys
 */

#include "AQNetworkTools.h"

namespace OPAQ
{
namespace AQNetworkTools
{

bool stationHasPollutant(Station* station, Pollutant& pollutant)
{
    auto& pols = station->getPollutants();
    auto iter = std::find_if(pols.begin(), pols.end(), [&pollutant] (const Pollutant* pol) {
        return pol->getId() == pollutant.getId();
    });

    return iter == pols.end();
}

}
}
