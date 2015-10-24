/*
 * Station.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys, maiheub
 */

#ifndef OPAQ_STATION_H_
#define OPAQ_STATION_H_

#include <string>
#include <vector>

#include "Pollutant.h"
#include "Point.h"

namespace OPAQ {
  /** Air quality station class.
   *  This class inherits from a simple point (with ID) and extends it with a 
   *  list of pollutants, a name (the station code), type and a description
   *  The class inherits from a Point. 
   *
   * \note Each station contains a list of pollutants it contains 
   */
  class Station: public Point {
  public:
    Station();
    virtual ~Station();

    /** Output streamer for the station class */
    friend std::ostream& operator<<(std::ostream& os, const Station& s );

    /**
     * Returns a vector with pointers to pollutant objects, indicats which
     * pollutants this station is measuring.
     */
    std::vector<Pollutant*> & getPollutants() { return pollutants; }

    /** Get the station name */
    std::string getName() const { return name; }

    /** Set the station name */
    void        setName( std::string name ) { this->name = name; }

    /** Get the station description */
    std::string getDescription() const { return desc; }

    /** Set the station description */
    void        setDescription( std::string desc ) { this->desc = desc; }

    /** Get the id for the meteo model that this station corresponds to.
	\note This is just a string or a tag indicating what meteo forecasts to take, 
              these can be e.g. LON_LAT string designating a certain mesoscale meteo
              model gridcel. This was inspired by the OVL matlab code which had per 
             station a certain tag such as 510_45 for ECMWF gridcell 51.0 N, 4.5 E
    */
    std::string getMeteoId() const { return meteoId; }

    /** Sets the meteo id, see remark under getMeteoId */
    void        setMeteoId( std::string id ) { this->meteoId = id; }

  private:
    std::vector<Pollutant *> pollutants;  //!< a list of pollutants measured by the station
    std::string              name;        //!< a station name (short code)
    std::string              desc;        //!< station description (full specification)
    std::string              meteoId;    //!< meteo model forecast id to connect to (typically gridcell or so)

    //TODO
    // add a station type, where the different possible types are configred by the 
    // main XML file..

  };
  
} /* namespace OPAQ */
#endif /* OPAQ_STATION_H_ */
