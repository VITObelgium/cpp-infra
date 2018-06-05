#include "OpaqRun.h"
#include "../PollutantManager.h"

namespace opaq {
namespace config {

using namespace infra;

OpaqRun::OpaqRun()
: _aggregation(Aggregation::None)
{
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
    auto iter = std::find_if(_plugins.begin(), _plugins.end(), [&](config::Plugin& plugin) {
        return plugin.name == pluginName;
    });

    if (iter == _plugins.end()) {
        throw BadConfigurationException("Plugin with name '{}' not found.", pluginName);
    }

    return *iter;
}

const Component& OpaqRun::getComponent(std::string_view componentName)
{
    auto iter = std::find_if(_components.begin(), _components.end(), [&](config::Component& comp) {
        return comp.name == componentName;
    });

    if (iter == _components.end()) {
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

std::string OpaqRun::getPollutantName() const
{
    return _pollutantName;
}

bool OpaqRun::pollutantIsSet() const noexcept
{
    return !_pollutantName.empty();
}

Aggregation::Type OpaqRun::getAggregation() const
{
    return _aggregation;
}

std::vector<chrono::date_time> OpaqRun::getBaseTimes() const
{
    return _baseTimes;
}

void OpaqRun::addBaseTime(const chrono::date_time& dt)
{
    _baseTimes.push_back(dt);
}

void OpaqRun::clearBaseTimes()
{
    _baseTimes.clear();
}

boost::optional<Component> OpaqRun::getNetworkProvider() const
{
    return _networkProvider;
}

boost::optional<Component> OpaqRun::getGridProvider() const
{
    return _gridProvider;
}

boost::optional<ForecastStage> OpaqRun::getForecastStage() const
{
    return _forecastStage;
}

boost::optional<MappingStage> OpaqRun::getMappingStage() const
{
    return _mappingStage;
}

void OpaqRun::setPollutantName(const std::string& name, const std::string& aggr)
{
    _pollutantName = name;
    _aggregation   = Aggregation::fromString(aggr);
}

void OpaqRun::setAggregation(const std::string& aggr)
{
    _aggregation = Aggregation::fromString(aggr);
}

void OpaqRun::setNetworkProvider(const Component& component)
{
    _networkProvider = component;
}

void OpaqRun::setGridProvider(const Component& component)
{
    _gridProvider = component;
}

void OpaqRun::setForecastStage(ForecastStage forecastStage)
{
    _forecastStage = forecastStage;
}

void OpaqRun::setMappingStage(MappingStage mappingStage)
{
    _mappingStage = mappingStage;
}
}
}
