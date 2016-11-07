#include "MappingStage.h"

#include "../Exceptions.h"
#include "../tools/ExceptionTools.h"

namespace opaq
{
namespace config
{

MappingStage::MappingStage(Component stations, Component obsProvider, std::vector<Component> models)
: _stations(stations)
, _obsProvider(obsProvider)
, _models(std::move(models))
{
}

Component MappingStage::getStationProvider() const
{
    return _stations;
}

Component MappingStage::getDataProvider() const
{
    return _obsProvider;
}

std::vector<Component> MappingStage::getModels() const
{
    return _models;
}

}
}