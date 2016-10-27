#include "ComponentManager.h"
#include "Exceptions.h"
#include "Logger.h"

#include "config.h"

namespace OPAQ
{

ComponentManager::ComponentManager(IEngine& engine, std::function<FactoryCallback(const std::string&, const std::string&)> cb)
: _loadPluginCb(cb)
, _engine(engine)
{
}

Component& ComponentManager::createGenericComponent(const std::string& componentName,
                                                    const std::string& pluginName,
                                                    TiXmlElement* configuration)
{
    // 1. check if the component wasn't created previously
    if (_instanceMap.find(componentName) != _instanceMap.end())
    {
        throw ComponentAlreadyExistsException("Component already exists: {}", componentName);
    }

    // 2. find the factory method for the plugin, create an instance and configure it
    auto component = createComponent(pluginName, componentName, configuration);

    // 3. store the instance in the instance map
    _instanceMap.emplace(componentName, std::move(component));
    // 4. and return it
    return *_instanceMap.at(componentName);
}

Component& ComponentManager::findComponent(const std::string& name)
{
    auto it = _instanceMap.find(name);
    if (it == _instanceMap.end())
    {
        throw ComponentNotFoundException("Component not found: {}", name);
    }

    return *it->second;
}

std::unique_ptr<Component> ComponentManager::createComponent(const std::string& pluginName, const std::string& componentName, TiXmlElement* configuration)
{
    auto it = _factoryMap.find(pluginName);
    if (it == _factoryMap.end())
    {
        throw PluginNotFoundException("Plugin not found: {}", pluginName);
    }

    auto& sink = Log::getConfiguration();
    std::unique_ptr<Component> component((it->second)(&sink));
    component->configure(configuration, componentName, _engine);
    return component;
}

void ComponentManager::loadPlugin(const std::string& pluginName, const std::string& filename)
{
    // 1. check if plugin was already loaded
    auto it = _factoryMap.find(pluginName);
    if (it != _factoryMap.end())
    {
        // Plugin already loaded
        return;
    }

    _factoryMap.emplace(pluginName, _loadPluginCb(pluginName, filename));
}

void ComponentManager::destroyComponent(const std::string& componentName)
{
    _instanceMap.erase(componentName);
}

void ComponentManager::destroyComponents()
{
    _instanceMap.clear();
}

}
