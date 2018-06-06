#pragma once

#include "Component.h"
#include "Exceptions.h"
#include "opaqconfig.h"

#include <functional>
#include <iostream>
#include <memory>
#include <unordered_map>

namespace infra {
class ConfigNode;
}

namespace opaq {

class IPluginFactory;

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
    ComponentManager(IEngine& engine, const IPluginFactory& pluginFactory);

    ComponentManager(ComponentManager&&)      = default;
    ComponentManager(const ComponentManager&) = delete;

    // throws (ComponentAlreadyExistsException, PluginNotFoundException, BadConfigurationException)
    template <typename T>
    T& createComponent(const std::string& componentName, const std::string& pluginName, const infra::ConfigNode& configuration)
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
        try {
            return &dynamic_cast<T&>(findComponent(componentName));
        } catch (NullPointerException&) {
            return nullptr;
        }
    }

    //void loadPlugin(const std::string& pluginName);
    void destroyComponent(const std::string& componentName);
    void destroyComponents();

private:
    // throws ComponentAlreadyExistsException, PluginNotFoundException, BadConfigurationException
    Component& createGenericComponent(const std::string& componentName, const std::string& pluginName, const infra::ConfigNode& configuration);

    // throws ComponentNotFoundException
    Component& findComponent(const std::string& name);

    // throw PluginNotFoundException, BadConfigurationException
    std::unique_ptr<Component> createComponent(const std::string& pluginName, const std::string& componentName, const infra::ConfigNode& configuration);
    std::unordered_map<std::string, std::unique_ptr<Component>> _instanceMap;

    IEngine& _engine;
    const IPluginFactory& _pluginFactory;
};
}
