#include "ComponentManagerDynamic.h"
#include "Exceptions.h"

#include <boost/dll/import.hpp>

namespace OPAQ
{

namespace dll = boost::dll;

ComponentManagerDynamic::ComponentManagerDynamic(IEngine& engine)
: ComponentManager(engine)
{
}

// Throws FailedToLoadPluginException PluginAlreadyLoadedException
void ComponentManagerDynamic::loadPlugin(const std::string& pluginName, const std::string& filename)
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

}
