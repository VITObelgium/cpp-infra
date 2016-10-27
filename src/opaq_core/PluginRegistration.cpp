#include "PluginRegistration.h"
#include "Exceptions.h"

#include <boost/dll/import.hpp>

namespace OPAQ
{

namespace dll = boost::dll;

void PluginRegistry::registerPlugin(const std::string& name, FactoryCallback cb)
{
    _registeredFactories.emplace(name, cb);
}

FactoryCallback PluginRegistry::getPluginFactory(const std::string& name)
{
    auto iter = PluginRegistry::instance()._registeredFactories.find(name);
    if (iter == PluginRegistry::instance()._registeredFactories.end())
    {
        throw RunTimeException("No plugin registered with name {}", name);
    }

    return iter->second;
}

FactoryCallback loadDynamicPlugin(const std::string& pluginName, const std::string& filename)
{
    Logger logger("Plugins");
    logger->info("Loading plugin {} from {}", pluginName, filename);

    typedef Component* (FactoryFunc)(LogConfiguration*);
    return dll::import<FactoryFunc>(filename.c_str(), "factory", boost::dll::load_mode::rtld_lazy);
}

FactoryCallback loadStaticPlugin(const std::string& pluginName, const std::string&)
{
    Logger logger("Plugins");
    logger->info("Loading plugin {}", pluginName);

    return PluginRegistry::instance().getPluginFactory(pluginName);
}

}

