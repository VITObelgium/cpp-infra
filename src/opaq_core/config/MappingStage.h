#pragma once

#include "GridType.h"
#include "Component.h"

#include <vector>

namespace opaq
{
namespace config
{

class MappingStage
{
public:
    MappingStage(GridType gridType, Component obsProvider, Component buffer, std::vector<Component> models);

    GridType getGridType() const;
    Component getDataProvider() const;
    Component getMappingBuffer() const;
    std::vector<Component> getModels() const;

private:
    GridType _gridType;
    Component _obsProvider;
    Component _buffer;
    std::vector<Component> _models;
};

}
}
