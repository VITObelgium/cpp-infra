#include "PluginRegistration.h"
#include "Exceptions.h"

#include <boost/dll/import.hpp>

namespace OPAQ
{

namespace dll = boost::dll;

std::map<std::string, FactoryCallback> PluginRegistry::_registeredFactories;

FactoryCallback loadDynamicPlugin(const std::string& pluginName, const std::string& filename)
{
    typedef Component* (FactoryFunc)(LogConfiguration*);
    return dll::import<FactoryFunc>(filename.c_str(), "factory", boost::dll::load_mode::rtld_lazy);
}

FactoryCallback loadStaticPlugin(const std::string& pluginName, const std::string&)
{
    auto iter = PluginRegistry::_registeredFactories.find(pluginName);
    if (iter == PluginRegistry::_registeredFactories.end())
    {
        throw RunTimeException("No plugin registered with name {}", pluginName);
    }

    return iter->second;
}

}

