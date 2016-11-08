#include "MappingStage.h"

#include "../Exceptions.h"
#include "../tools/ExceptionTools.h"

namespace opaq
{
namespace config
{

MappingStage::MappingStage(Component stations, Component obsProvider, Component buffer, std::vector<Component> models)
: _stations(stations)
, _obsProvider(obsProvider)
, _buffer(buffer)
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

Component MappingStage::getMappingBuffer() const
{
    return _buffer;
}

std::vector<Component> MappingStage::getModels() const
{
    return _models;
}

}
}