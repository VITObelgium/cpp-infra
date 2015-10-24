#ifndef OPAQ_POLLUTANT_H_
#define OPAQ_POLLUTANT_H_

#include <iostream>
#include <string>
#include <vector>

#include <tinyxml.h>

namespace OPAQ {


  /**
     Class to represent a pollutant in OPAQ
     The pollutant definition is normally held in a special section in the configuration file
     which defines what pollutants are available to OPAQ. 
  */
  class Pollutant {
  public:
    /** Empty constructor */
    Pollutant();

    /** 
	Constructor from an XML element
	\param el A const pointer to the TiXmlELement holding the pollutant definitoin
    */
    Pollutant( TiXmlElement const *el );

    virtual ~Pollutant();

    /** Writes out the pollutant information to the ostream */
    friend std::ostream& operator<<(std::ostream& os, const Pollutant& s );
    
    /** Converts the pollutant to a string, making use of the outputstreamer */
    std::string toString(); 
    
    /** returns the ID of the pollutant */
    long getId() const { return id; }  

    /**  sets the ID of the pollutant */
    void setId(long id) { this->id = id; } 

    /** returns the name of the pollutant */
    std::string getName() const { return name; } 

    /** sets the name of the pollutant */
    void setName(std::string name) { this->name = name; }

    /** 
	returns the units of the pollutant
	\note that this is just a string, there is no real meaning or 
	functionality assigned to the units (yet)
    */
    std::string getUnit() const { return unit; }

    /** sets the units, see note under setUnit  */
    void setUnit(std::string unit) { this->unit = unit; } 

    /** returns the description */
    std::string getDescription() const { return desc; }

    /** sets the pollutant description */
    void setDescription(std::string desc) { this->desc = desc; }
    
  private:
    long id;
    std::string name;
    std::string unit;
    std::string desc;
  };
  
}
#endif /* #ifndef OPAQ_POLLUTANT_H_ */
