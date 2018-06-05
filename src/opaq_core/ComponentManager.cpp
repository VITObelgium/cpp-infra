#include "ComponentManager.h"
#include "Exceptions.h"
#include "PluginFactoryInterface.h"

#include "config.h"

namespace opaq {

using namespace infra;

ComponentManager::ComponentManager(IEngine& engine, const IPluginFactory& pluginFactory)
: _engine(engine)
, _pluginFactory(pluginFactory)
{
}

Component& ComponentManager::createGenericComponent(const std::string& componentName, const std::string& pluginName, const ConfigNode& configuration)
{
    // 1. check if the component wasn't created previously
    if (_instanceMap.find(componentName) != _instanceMap.end()) {
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
    if (it == _instanceMap.end()) {
        throw ComponentNotFoundException("Component not found: {}", name);
    }

    return *it->second;
}

std::unique_ptr<Component> ComponentManager::createComponent(const std::string& pluginName, const std::string& componentName, const ConfigNode& configuration)
{
    auto component = _pluginFactory.createPlugin(pluginName);
    component->configure(configuration, componentName, _engine);
    return component;
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
