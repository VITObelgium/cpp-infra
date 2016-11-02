#ifndef OPAQ_POLLUTANT_H_
#define OPAQ_POLLUTANT_H_

#include <iostream>
#include <string>
#include <vector>

#include <tinyxml.h>

namespace opaq
{

/**
     Class to represent a pollutant in OPAQ
     The pollutant definition is normally held in a special section in the configuration file
     which defines what pollutants are available to OPAQ. 
  */
class Pollutant
{
public:
    Pollutant();
    Pollutant(long id, std::string name, std::string unit, std::string desc);

    /** 
    Constructor from an XML element
    \param el A const pointer to the TiXmlELement holding the pollutant definitoin
    */
    Pollutant(TiXmlElement const* el);

    /** Writes out the pollutant information to the ostream */
    friend std::ostream& operator<<(std::ostream& os, const Pollutant& s);

    /** Converts the pollutant to a string, making use of the outputstreamer */
    std::string toString() const;

    /** returns the ID of the pollutant */
    long getId() const { return _id; }

    /** returns the name of the pollutant */
    std::string getName() const { return _name; }

    /** 
    returns the units of the pollutant
    \note that this is just a string, there is no real meaning or 
    functionality assigned to the units (yet)
    */
    std::string getUnit() const { return _unit; }

    /** returns the description */
    std::string getDescription() const { return _desc; }

private:
    long _id;
    std::string _name;
    std::string _unit;
    std::string _desc;
};
}
#endif /* #ifndef OPAQ_POLLUTANT_H_ */
