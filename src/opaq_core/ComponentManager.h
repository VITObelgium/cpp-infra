/*
 *      Created on: Dec 20, 2013
 *      Author: vlooys
 *
 */

#ifndef COMPONENTMANAGER_H_
#define COMPONENTMANAGER_H_

#include "Component.h"
#include "OpaqDllExports.h"
#include "Logger.h"

#include <tinyxml.h>
#include <map>
#include <memory>

/**
 * \brief Macro to register a class as being an OPAQ component.
 * When writing new components, a user should add this macro statement to 
 * the implementation source file. Dont forget to include a correct namespace
 * if you want to have the plugin to have it's own namespace. 
 *
 *   Example use: OPAQ_REGISTER_PLUGIN(OPAQ::ExampleComponent);
 */
#define OPAQ_REGISTER_PLUGIN(TYPE)		\
    OPAQ_DLL_API OPAQ::Component * factory (void* sink) {		\
      Log::initLogger(*reinterpret_cast<std::shared_ptr<spdlog::sinks::sink>*>(sink)); \
      return new TYPE();			\
  }

/**
   The OPAQ namespace collects all the classes and the functions which reside in the OPAQ framework.
*/
namespace OPAQ {
  // forward declarations
  class IEngine;
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
    typedef std::map<std::string, Component*(*)(void*)> factoryMapType;
    
    ComponentManager(IEngine& engine);
    virtual ~ComponentManager();
    
    // throws FailedToLoadPluginException, PluginAlreadyLoadedException
    void loadPlugin(std::string &pluginName, std::string &filename);
    
    // throws (ComponentAlreadyExistsException, PluginNotFoundException, BadConfigurationException)
    template<typename T> T * createComponent(std::string &componentName, std::string &pluginName, TiXmlElement* configuration) {
        Component * component = createGenericComponent(componentName, pluginName, configuration);
        return dynamic_cast<T*>(component);
    }
    
    // throws ComponentNotFoundException
    template<typename T> T * getComponent(std::string &componentName) {
      Component * component = findComponent(componentName);
      return dynamic_cast<T*>(component);
    }
    
    void destroyComponent(std::string &componentName);
    
  private:
    instanceMapType instanceMap;
    factoryMapType factoryMap;
    IEngine& _engine;
    
    ComponentManager(ComponentManager const&);	// no implementation of copy constructor for singleton
    void operator=(ComponentManager const&);	// no implementation of assignment operator for singleton
    

    // throws ComponentAlreadyExistsException, PluginNotFoundException, BadConfigurationException
    Component* createGenericComponent (std::string &componentName, std::string &pluginName, TiXmlElement* configuration);

    // throws ComponentNotFoundException
    Component* findComponent (std::string &name);

    // throw PluginNotFoundException, BadConfigurationException
    Component * createComponent (std::string &pluginName, TiXmlElement* configuration);
  };
  
} /* namespace OPAQ */

#endif /* COMPONENTMANAGER_H_ */
