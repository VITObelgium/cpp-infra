#pragma once

#include <string>

class TiXmlElement;

namespace OPAQ
{

class IEngine;
class ComponentManager;

/**
     \brief The base class for an OPAQ component
     \author Stijn Van Looy

     This class is the base class for an OPAQ component. It provides a pure virtual 
     configure function, which each of the derived classes need to implement.
  */
class Component
{

    /**
       Declare the component manager as a friend as it has to be able to 
       set the name of the component via private setName method
    */
    friend class ComponentManager;

public:
    virtual ~Component() = default;

    /**
     * Configure the component.
     * @param configuration pointer to the XML element holding the components configuration
     * @param pollutantMgr the pollutant manager instance
     * @throws BadConfigurationException if the component failed to configure using the provided configuration
     */
    virtual void configure(TiXmlElement* configuration, IEngine& engine) = 0;

    std::string getName() const noexcept;

private:
    void setName(const std::string& componentName);
    
    std::string _name; //!< the component name (from the XML)
};

}
