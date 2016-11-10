#pragma once

#include "config.h"
#include "Component.h"
#include "Logger.h"
#include "Exceptions.h"

#include <map>
#include <iostream>
#include <memory>
#include <functional>

namespace opaq
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

using FactoryCallback = std::function<Component*(LogConfiguration*)>;

class ComponentManager
{
public:
    ComponentManager(IEngine& engine, std::function<FactoryCallback(const std::string&, const std::string&)> cb);

    ComponentManager(ComponentManager&&) = default;
    ComponentManager(const ComponentManager&) = delete;

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

    template <typename T>
    T* getOptionalComponent(const std::string& componentName)
    {
        try
        {
            return &dynamic_cast<T&>(findComponent(componentName));
        }
        catch (NullPointerException&)
        {
            return nullptr;
        }
    }

    void loadPlugin(const std::string& pluginName, const std::string& filename);
    void destroyComponent(const std::string& componentName);
    void destroyComponents();

private:
    // throws ComponentAlreadyExistsException, PluginNotFoundException, BadConfigurationException
    Component& createGenericComponent(const std::string& componentName, const std::string& pluginName, TiXmlElement* configuration);

    // throws ComponentNotFoundException
    Component& findComponent(const std::string& name);

    // throw PluginNotFoundException, BadConfigurationException
    std::unique_ptr<Component> createComponent(const std::string& pluginName, const std::string& componentName, TiXmlElement* configuration);

    std::function<FactoryCallback(const std::string&, const std::string&)> _loadPluginCb;
    // Factory map must occur before instance map, destroying the factory function causes the dll to be unloaded
    // The instance map has to be destroyed before the dll unload
    std::map<std::string, FactoryCallback> _factoryMap;
    std::map<std::string, std::unique_ptr<Component>> _instanceMap;
    IEngine& _engine;
};

}
