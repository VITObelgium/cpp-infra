#include "OpaqRun.h"
#include "../PollutantManager.h"

namespace opaq
{
namespace Config
{

OpaqRun::OpaqRun()
: _logger("OPAQ::Config::OpaqRun")
, _aggregation(Aggregation::None)
, _networkProvider(nullptr)
, _gridProvider(nullptr)
{
}

void OpaqRun::clear()
{
    _plugins.clear();
    _components.clear();
    _baseTimes.clear();

    _networkProvider.reset();
    _gridProvider.reset();
    _forecastStage.reset();
    _mappingStage.reset();
}

void OpaqRun::addPlugin(const Plugin& plugin)
{
    _plugins.push_back(plugin);
}

void OpaqRun::addComponent(const Component& component)
{
    _components.push_back(component);
}

Plugin OpaqRun::getPlugin(const std::string& pluginName)
{
    auto iter = std::find_if(_plugins.begin(), _plugins.end(), [&](Config::Plugin& plugin) {
        return plugin.name == pluginName;
    });

    if (iter == _plugins.end())
    {
        throw BadConfigurationException("Plugin with name '{}' not found.", pluginName);
    }

    return *iter;
}

const Component& OpaqRun::getComponent(const std::string& componentName)
{
    auto iter = std::find_if(_components.begin(), _components.end(), [&](Config::Component& comp) {
        return comp.name == componentName;
    });

    if (iter == _components.end())
    {
        throw BadConfigurationException("Component with name '{}' not found.", componentName);
    }

    return *iter;
}

std::vector<Plugin> OpaqRun::getPlugins()
{
    return _plugins;
}

std::vector<Component> OpaqRun::getComponents()
{
    return _components;
}

}
}
