/*
 *
 *  Created on: Dec 20, 2013
 *      Author: vlooys
 */

#include "ComponentManager.h"
#include "Exceptions.h"
#include "Logger.h"
#include <dlfcn.h>

namespace OPAQ
{

ComponentManager::ComponentManager(IEngine& engine)
: _engine(engine)
{
}

// Throws FailedToLoadPluginException PluginAlreadyLoadedException
void ComponentManager::loadPlugin(const std::string& pluginName, const std::string& filename)
{
    // 1. check if plugin was already loaded
    FactoryMapType::iterator it = factoryMap.find(pluginName);
    if (it != factoryMap.end())
    {
        throw RunTimeException("There already exists a plugin with name {}", pluginName);
    }

    // 2. load plugin
    void* handle = dlopen(filename.c_str(), RTLD_LAZY);
    if (!handle)
    {
        auto* error = dlerror();
        throw FailedToLoadPluginException("Failed to load library with name '{}' from file with name '{}': {}", pluginName, filename, (error ? error : ""));
    }
    //BM funky function handle casting going on here :-)
    auto factory = reinterpret_cast<FactoryFunc>(dlsym(handle, "factory"));
    if (factory == nullptr)
    {
        auto* error = dlerror();
        throw FailedToLoadPluginException("Failed to fetch factory symbol from library with name '{}' in file with name '{}': {}", pluginName, filename, (error ? error : ""));
    }

    // Create the _logger in the application and not in the dll memory space
    factoryMap.emplace(pluginName, factory);
}

// Throws ComponentAlreadyExistsException PluginNotFoundException BadConfigurationException
Component& ComponentManager::createGenericComponent(const std::string& componentName,
                                                    const std::string& pluginName,
                                                    TiXmlElement* configuration)
{
    // 1. check if the component wasn't created previously
    if (instanceMap.find(componentName) != instanceMap.end())
    {
        throw ComponentAlreadyExistsException("Component already exists: {}", componentName);
    }

    // 2. find the factory method for the plugin, create an instance and configure it
    auto component = createComponent(pluginName, configuration);

    // 2.1 store the component name in the component, to be returned by getName();
    component->setName(componentName);

    // 3. store the instance in the instance map
    instanceMap.emplace(componentName, std::move(component));
    // 4. and return it
    return *instanceMap.at(componentName);
}

// Throws ComponentNotFoundException
Component& ComponentManager::findComponent(const std::string& name)
{
    auto it = instanceMap.find(name);
    if (it == instanceMap.end())
    {
        throw ComponentNotFoundException("Component not found: {}", name);
    }

    return *it->second;
}

// Throws PluginNotFoundException BadConfigurationException
std::unique_ptr<Component> ComponentManager::createComponent(const std::string& pluginName, TiXmlElement* configuration)
{
    auto it = factoryMap.find(pluginName);
    if (it == factoryMap.end())
    {
        throw PluginNotFoundException("Plugin not found: {}", pluginName);
    }

    auto& sink = Log::getConfiguration();
    std::unique_ptr<Component> component(it->second(&sink));
    component->configure(configuration, _engine);
    return component;
}

void ComponentManager::destroyComponent(const std::string& componentName)
{
    instanceMap.erase(componentName);
}
}
