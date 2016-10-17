#include "ComponentManagerStatic.h"
#include "Exceptions.h"
#include "Logger.h"

namespace OPAQ
{

ComponentManagerStatic::FactoryMapType ComponentManagerStatic::_registeredFactories;

ComponentManagerStatic::ComponentManagerStatic(IEngine& engine)
: ComponentManager(engine)
{
}

// Throws FailedToLoadPluginException PluginAlreadyLoadedException
void ComponentManagerStatic::loadPlugin(const std::string& pluginName, const std::string&)
{
    // 1. check if plugin was already loaded
    if (factoryMap.find(pluginName) != factoryMap.end())
    {
        throw RunTimeException("There already exists a plugin with name {}", pluginName);
    }

    // 2. load plugin function from registry
    auto iter = _registeredFactories.find(pluginName);
    if (iter != _registeredFactories.end())
    {
        throw RunTimeException("No plugin registered with name {}", pluginName);
    }

    factoryMap.emplace(*iter);
}

void ComponentManagerStatic::registerPlugin(const std::string& name, FactoryCallback cb)
{
    _registeredFactories.emplace(name, cb);
}

}
