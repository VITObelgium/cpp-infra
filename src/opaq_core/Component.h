#ifndef COMPONENT_H_
#define COMPONENT_H_

#include <tinyxml.h>
#include "Exceptions.h"

namespace OPAQ {
  
  /**
     \brief The base class for an OPAQ component
     \author Stijn Van Looy

     This class is the base class for an OPAQ component. It provides a pure virtual 
     configure function, which each of the dauther classes need to implement.
  */
  class Component {
  public:
    Component(); 
    virtual ~Component(); 
    
    /**
     * Configure the component.
     * @param configuration pointer to the XML element holding the components configuration
     * @throws BadConfigurationException if the component failed to configure using the provided configuration
     */
    virtual void configure (TiXmlElement * configuration) throw (BadConfigurationException) = 0;
    
  };
  
} /* namespace OPAQ */
#endif /* COMPONENT_H_ */
