#ifndef COMPONENT_H_
#define COMPONENT_H_

#include <tinyxml.h>
#include "Exceptions.h"

namespace OPAQ {

  class ComponentManager; 
  
  /**
     \brief The base class for an OPAQ component
     \author Stijn Van Looy

     This class is the base class for an OPAQ component. It provides a pure virtual 
     configure function, which each of the dauther classes need to implement.
  */
  class Component {

    /**
       Declare the component manager as a friend as it has to be able to 
       set the name of the component via private setName method
    */
    friend class ComponentManager; 
    
  public:
    Component(); 
    virtual ~Component(); 
    
    /**
     * Configure the component.
     * @param configuration pointer to the XML element holding the components configuration
     * @throws BadConfigurationException if the component failed to configure using the provided configuration
     */
    virtual void configure (TiXmlElement * configuration) = 0;

    const std::string & getName( void ){ return name; }

  private:
    void setName( const std::string & componentName ){ name = componentName; }
    std::string name; //!< the component name (from the XML) 
    
  };
  
} /* namespace OPAQ */
#endif /* COMPONENT_H_ */
