/*
* Station.h
*
*  Created on: Jan 9, 2014
*      Author: vlooys, maiheub
*/

#pragma once

#include <string>
#include <vector>

#include "Point.h"
#include "Pollutant.h"

namespace opaq
{
/** Air quality station class.
*  This class inherits from a simple point (with ID) and extends it with a
*  list of pollutants, a name (the station code), type and a description
*  The class inherits from a Point.
*
* \note Each station contains a list of pollutants it contains
*/
class Station : public Point
{
public:
    Station(std::string name, std::string desc, std::string meteoId);
    Station(long id, double x, double y, double z, std::string name, std::string desc, std::string meteoId);

    friend std::ostream& operator<<(std::ostream& os, const Station& s);

    void addPollutant(const Pollutant& p);

    const std::string& getName() const;
    const std::string& getDescription() const;

    /** Get the id for the meteo model that this station corresponds to.
    \note This is just a string or a tag indicating what meteo forecasts to take,
    these can be e.g. LON_LAT string designating a certain mesoscale meteo
    model gridcel. This was inspired by the OVL matlab code which had per
    station a certain tag such as 510_45 for ECMWF gridcell 51.0 N, 4.5 E
    */
    const std::string& getMeteoId() const;

    bool measuresPollutant(const Pollutant& pol) const noexcept;

    bool operator==(const Station& other) const noexcept;

private:
    std::vector<Pollutant> _pollutants;  //!< a list of pollutants measured by the station
    std::string _name;                   //!< a station name (short code)
    std::string _desc;                   //!< station description (full specification)
    std::string _meteoId;                //!< meteo model forecast id to connect to (typically gridcell or so)
};

}