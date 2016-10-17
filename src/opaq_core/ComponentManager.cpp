#include "ComponentManager.h"
#include "Exceptions.h"
#include "Logger.h"

#include <boost/dll/import.hpp>

namespace OPAQ
{

namespace dll = boost::dll;

ComponentManager::ComponentManager(IEngine& engine)
: _engine(engine)
{
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
    auto component = createComponent(pluginName, componentName, configuration);

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
std::unique_ptr<Component> ComponentManager::createComponent(const std::string& pluginName, const std::string& componentName, TiXmlElement* configuration)
{
    auto it = factoryMap.find(pluginName);
    if (it == factoryMap.end())
    {
        throw PluginNotFoundException("Plugin not found: {}", pluginName);
    }

    auto& sink = Log::getConfiguration();
    std::unique_ptr<Component> component((it->second)(&sink));
    component->configure(configuration, componentName, _engine);
    return component;
}

void ComponentManager::destroyComponent(const std::string& componentName)
{
    instanceMap.erase(componentName);
}
}
