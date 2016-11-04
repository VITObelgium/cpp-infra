#include "MappingStage.h"

#include "../Exceptions.h"
#include "../tools/ExceptionTools.h"

namespace opaq
{
namespace config
{

MappingStage::MappingStage(Component stations, std::vector<Component> models)
: _stations(stations)
, _models(std::move(models))
{
}

Component MappingStage::getStationProvider() const
{
    return _stations;
}

std::vector<Component> MappingStage::getModels() const
{
    return _models;
}

}
}