#pragma once

#include <string>

class TiXmlElement;

namespace opaq
{

class IEngine;

/**
     \brief The base class for an OPAQ component
     \author Stijn Van Looy

     This class is the base class for an OPAQ component. It provides a pure virtual
     configure function, which each of the derived classes need to implement.
  */
class Component
{
public:
    virtual ~Component() = default;

    /**
     * Configure the component.
     * @param configuration pointer to the XML element holding the components configuration
     * @param pollutantMgr the pollutant manager instance
     * @param componentName the name of the component
     * @throws BadConfigurationException if the component failed to configure using the provided configuration
     */
    virtual void configure(TiXmlElement* configuration, const std::string& componentName, IEngine& engine) = 0;

    std::string getName() const noexcept;

protected:
    void setName(const std::string& componentName);

private:
    std::string _name; //!< the component name (from the XML)
};

}
