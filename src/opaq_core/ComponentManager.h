/*
 *      Created on: Dec 20, 2013
 *      Author: vlooys
 *
 */

#pragma once

#include "Component.h"
#include "Logger.h"
#include "OpaqDllExports.h"

#include <map>
#include <memory>
#include <tinyxml.h>

/**
 * \brief Macro to register a class as being an OPAQ component.
 * When writing new components, a user should add this macro statement to 
 * the implementation source file. Dont forget to include a correct namespace
 * if you want to have the plugin to have it's own namespace. 
 *
 *   Example use: OPAQ_REGISTER_PLUGIN(OPAQ::ExampleComponent);
 */
#define OPAQ_REGISTER_PLUGIN(TYPE)                                                       \
    OPAQ_DLL_API OPAQ::Component* factory(LogConfiguration* logConfig)                   \
    {                                                                                    \
        Log::initLogger(*logConfig);                                                     \
        return new TYPE();                                                               \
    }

/**
   The OPAQ namespace collects all the classes and the functions which reside in the OPAQ framework.
*/
namespace OPAQ
{
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
   * \brief class for managing the components. 
   * \author Stijn Van Looy
   *
   *  Class to manage the components in OPAQ. It provides functionality to discover and 
   *  create instances of the different OPAQ plugins which derive from OPAQ::Component. 
   *
   *  This factory class was started from and loosely inspired by:
   *  http://stackoverflow.com/questions/582331/is-there-a-way-to-instantiate-objects-from-a-string-holding-their-class-name
   *
   */
class ComponentManager
{
public:
    typedef Component* (*FactoryFunc)(LogConfiguration*);
    typedef std::map<std::string, Component*> InstanceMapType;
    typedef std::map<std::string, FactoryFunc> FactoryMapType;

    ComponentManager(IEngine& engine);
    virtual ~ComponentManager();

    // throws FailedToLoadPluginException, PluginAlreadyLoadedException
    void loadPlugin(const std::string& pluginName, const std::string& filename);

    // throws (ComponentAlreadyExistsException, PluginNotFoundException, BadConfigurationException)
    template <typename T>
    T* createComponent(const std::string& componentName, const std::string& pluginName, TiXmlElement* configuration)
    {
        Component* component = createGenericComponent(componentName, pluginName, configuration);
        return dynamic_cast<T*>(component);
    }

    // throws ComponentNotFoundException
    template <typename T>
    T* getComponent(const std::string& componentName)
    {
        Component* component = findComponent(componentName);
        return dynamic_cast<T*>(component);
    }

    void destroyComponent(const std::string& componentName);

private:
    InstanceMapType instanceMap;
    FactoryMapType factoryMap;
    IEngine& _engine;

    ComponentManager(ComponentManager const&); // no implementation of copy constructor for singleton
    void operator=(ComponentManager const&);   // no implementation of assignment operator for singleton

    // throws ComponentAlreadyExistsException, PluginNotFoundException, BadConfigurationException
    Component* createGenericComponent(const std::string& componentName, const std::string& pluginName, TiXmlElement* configuration);

    // throws ComponentNotFoundException
    Component* findComponent(const std::string& name);

    // throw PluginNotFoundException, BadConfigurationException
    Component* createComponent(const std::string& pluginName, TiXmlElement* configuration);
};

}
