#include "PluginRegistration.h"
#include "Exceptions.h"

namespace opaq {

void PluginRegistry::registerPlugin(const std::string& name, FactoryCallback cb)
{
    _registeredFactories.emplace(name, cb);
}

FactoryCallback PluginRegistry::getPluginFactory(const std::string& name)
{
    auto iter = PluginRegistry::instance()._registeredFactories.find(name);
    if (iter == PluginRegistry::instance()._registeredFactories.end()) {
        throw RunTimeException("No plugin registered with name {}", name);
    }

    return iter->second;
}

FactoryCallback loadStaticPlugin(const std::string& pluginName)
{
    Logger logger("Plugins");
    logger->info("Loading plugin {}", pluginName);

    return PluginRegistry::instance().getPluginFactory(pluginName);
}

}
