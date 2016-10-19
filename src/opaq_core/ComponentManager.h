#pragma once

#include "config.h"
#include "Component.h"
#include "Logger.h"
#include "OpaqDllExports.h"

#include <map>
#include <memory>
#include <functional>

namespace OPAQ
{

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
    ComponentManager(IEngine& engine);

    // throws (ComponentAlreadyExistsException, PluginNotFoundException, BadConfigurationException)
    template <typename T>
    T& createComponent(const std::string& componentName, const std::string& pluginName, TiXmlElement* configuration)
    {
        return dynamic_cast<T&>(createGenericComponent(componentName, pluginName, configuration));
    }

    // throws ComponentNotFoundException
    template <typename T>
    T& getComponent(const std::string& componentName)
    {
        return dynamic_cast<T&>(findComponent(componentName));
    }

    virtual void loadPlugin(const std::string& pluginName, const std::string& filename) = 0;
    void destroyComponent(const std::string& componentName);

    void registerPlugim();

protected:
    typedef Component* (FactoryFunc)(LogConfiguration*);
    typedef std::function<Component*(LogConfiguration*)> FactoryCallback;
    typedef std::map<std::string, std::unique_ptr<Component>> InstanceMapType;
    typedef std::map<std::string, FactoryCallback> FactoryMapType;

    // Factory map must occur before instance map, destroying the factory function causes the dll to be unloaded
    // The instance map has to be destroyed before the dll unload
    FactoryMapType factoryMap;
    InstanceMapType instanceMap;
    IEngine& _engine;

    // throws ComponentAlreadyExistsException, PluginNotFoundException, BadConfigurationException
    Component& createGenericComponent(const std::string& componentName, const std::string& pluginName, TiXmlElement* configuration);

    // throws ComponentNotFoundException
    Component& findComponent(const std::string& name);

    // throw PluginNotFoundException, BadConfigurationException
    std::unique_ptr<Component> createComponent(const std::string& pluginName, const std::string& componentName, TiXmlElement* configuration);
};
}

#ifdef STATIC_PLUGINS
#define OPAQ_REGISTER_PLUGIN(TYPE) \
  ;
//OPAQ::ComponentManagerStatic::registerPlugin(#TYPE, [] (LogConfiguration*) { return new TYPE(); });
#else
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

#endif

