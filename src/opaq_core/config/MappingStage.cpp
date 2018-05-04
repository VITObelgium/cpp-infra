#include "MappingStage.h"

#include "../Exceptions.h"
#include "../tools/ExceptionTools.h"

namespace opaq
{
namespace config
{

MappingStage::MappingStage(GridType gridType, Component obsProvider, Component buffer, std::vector<Component> models)
: _gridType(std::move(gridType))
, _obsProvider(std::move(obsProvider))
, _buffer(std::move(buffer))
, _models(std::move(models))
{
}

GridType MappingStage::getGridType() const
{
    return _gridType;
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