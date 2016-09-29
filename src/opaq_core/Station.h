/*
 * Station.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys, maiheub
 */

#pragma once

#include <string>
#include <vector>

#include "Point.h"
#include "Pollutant.h"

namespace OPAQ
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
    /** Output streamer for the station class */
    friend std::ostream& operator<<(std::ostream& os, const Station& s);

    /**
     * Returns a vector with pointers to pollutant objects, indicats which
     * pollutants this station is measuring.
     */
    std::vector<Pollutant*>& getPollutants() { return _pollutants; }

    /** Get the station name */
    std::string getName() const { return _name; }

    /** Set the station name */
    void setName(const std::string& name) { _name = name; }

    /** Get the station description */
    std::string getDescription() const { return _desc; }

    /** Set the station description */
    void setDescription(const std::string& desc) { _desc = desc; }

    /** Get the id for the meteo model that this station corresponds to.
  \note This is just a string or a tag indicating what meteo forecasts to take,
              these can be e.g. LON_LAT string designating a certain mesoscale meteo
              model gridcel. This was inspired by the OVL matlab code which had per
             station a certain tag such as 510_45 for ECMWF gridcell 51.0 N, 4.5 E
    */
    std::string getMeteoId() const { return _meteoId; }

    /** Sets the meteo id, see remark under getMeteoId */
    void setMeteoId(const std::string& id) { _meteoId = id; }

private:
    std::vector<Pollutant*> _pollutants; //!< a list of pollutants measured by the station
    std::string _name;                   //!< a station name (short code)
    std::string _desc;                   //!< station description (full specification)
    std::string _meteoId;                //!< meteo model forecast id to connect to (typically gridcell or so)
};

}
