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

FactoryCallback loadDynamicPlugin(const std::string& pluginName, const std::string& filename)
{
    typedef Component* (FactoryFunc)(LogConfiguration*);
    return dll::import<FactoryFunc>(filename.c_str(), "factory", boost::dll::load_mode::rtld_lazy);
}

FactoryCallback loadStaticPlugin(const std::string& pluginName, const std::string&)
{
    auto iter = PluginRegistry::instance()._registeredFactories.find(pluginName);
    if (iter == PluginRegistry::instance()._registeredFactories.end())
    {
        throw RunTimeException("No plugin registered with name {}", pluginName);
    }

    return iter->second;
}

}

