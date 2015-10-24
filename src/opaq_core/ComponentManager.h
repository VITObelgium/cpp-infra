/*
 *      Created on: Dec 20, 2013
 *      Author: vlooys
 *
 */

#ifndef COMPONENTMANAGER_H_
#define COMPONENTMANAGER_H_

#include "Component.h"
#include <tinyxml.h>
#include <map>

/**
 * \brief Macro to register a class as being an OPAQ component.
 * When writing new components, a user should add this macro statement to 
 * the implementation source file. Dont forget to include a correct namespace
 * if you want to have the plugin to have it's own namespace. 
 *
 *   Example use: OPAQ_REGISTER_PLUGIN(OPAQ::ExampleComponent);
 */
#define OPAQ_REGISTER_PLUGIN(TYPE)		\
  extern "C" {					\
    OPAQ::Component * factory () {		\
      return new TYPE();			\
    }						\
  }

/**
   The OPAQ namespace collects all the classes and the functions which reside in the OPAQ framework.
*/
namespace OPAQ {

  // forward declarations
  class Component;
  class FailedToLoadPluginException;
  class PluginAlreadyLoadedException;
  class ComponentAlreadyExistsException;
  class ComponentNotFoundException;
  class PluginNotFoundException;
  class BadConfigurationException;

  /** 
   * \brief Singleton class for managing the components. 
   * \author Stijn Van Looy
   *
   *  Class to manage the components in OPAQ. It provides functionality to discover and 
   *  create instances of the different OPAQ plugins which derive from OPAQ::Component. 
   *
   *  This factory class was started from and loosely inspired by:
   *  http://stackoverflow.com/questions/582331/is-there-a-way-to-instantiate-objects-from-a-string-holding-their-class-name
   *
   *  Depends on tinyxml
   *  	http://www.grinninglizard.com/tinyxml/
   *  	in Ubuntu: sudo aptitude install libtinyxml-dev
   */
  class ComponentManager {
  public:
    typedef std::map<std::string, Component*> instanceMapType;
    typedef std::map<std::string, Component*(*)()> factoryMapType;
    
    static ComponentManager * getInstance();
    
    virtual ~ComponentManager();
    
    void loadPlugin(std::string &pluginName, std::string &filename)
      throw (FailedToLoadPluginException, PluginAlreadyLoadedException);
    
    template<typename T> T * createComponent(std::string &componentName,
					     std::string &pluginName, TiXmlElement * configuration)
      throw (ComponentAlreadyExistsException, PluginNotFoundException, BadConfigurationException) {
      Component * component = createGenericComponent(componentName, pluginName, configuration);
      return dynamic_cast<T*>(component);
    }
    
    template<typename T> T * getComponent(std::string &componentName)
      throw (ComponentNotFoundException) {
      Component * component = findComponent(componentName);
      return dynamic_cast<T*>(component);
    }
    
    void destroyComponent(std::string &componentName);
    
  private:
    instanceMapType instanceMap;
    factoryMapType factoryMap;
    
    ComponentManager();
    ComponentManager(ComponentManager const&);	// no implementation of copy constructor for singleton
    void operator=(ComponentManager const&);	// no implementation of assignment operator for singleton
    

    Component * createGenericComponent (std::string &componentName,
					std::string &pluginName, TiXmlElement * configuration)
      throw (ComponentAlreadyExistsException, PluginNotFoundException, BadConfigurationException);

    Component * findComponent (std::string &name) throw (ComponentNotFoundException);

    Component * createComponent (std::string &pluginName, TiXmlElement * configuration)
      throw (PluginNotFoundException, BadConfigurationException);
  };
  
} /* namespace OPAQ */

#endif /* COMPONENTMANAGER_H_ */
