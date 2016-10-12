/*
 *
 *  Created on: Dec 20, 2013
 *      Author: vlooys
 */

#include "ComponentManager.h"
#include "Exceptions.h"
#include "Logger.h"

#include <boost/dll/import.hpp>         // for dll::import
#include <boost/dll/shared_library.hpp> // for dll::shared_library

namespace OPAQ
{

namespace dll = boost::dll;

ComponentManager::ComponentManager(IEngine& engine)
: _engine(engine)
{
}

// Throws FailedToLoadPluginException PluginAlreadyLoadedException
void ComponentManager::loadPlugin(const std::string& pluginName, const std::string& filename)
{
    // 1. check if plugin was already loaded
    auto it = factoryMap.find(pluginName);
    if (it != factoryMap.end())
    {
        throw RunTimeException("There already exists a plugin with name {}", pluginName);
    }

    // 2. load plugin function
    auto factoryFunc = dll::import<FactoryFunc>(filename.c_str(), "factory", boost::dll::load_mode::rtld_lazy);
    factoryMap.emplace(pluginName, factoryFunc);
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
    std::unique_ptr<Component> component((it->second)(&sink));
    component->configure(configuration, _engine);
    return component;
}

void ComponentManager::destroyComponent(const std::string& componentName)
{
    instanceMap.erase(componentName);
}
}
