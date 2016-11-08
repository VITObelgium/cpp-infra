#pragma once

#include "Component.h"

#include <vector>

namespace opaq
{
namespace config
{

class MappingStage
{
public:
    MappingStage(Component stations, Component obsProvider, Component buffer, std::vector<Component> models);

    Component getStationProvider() const;
    Component getDataProvider() const;
    Component getMappingBuffer() const;
    std::vector<Component> getModels() const;

private:
    Component _stations;
    Component _obsProvider;
    Component _buffer;
    std::vector<Component> _models;
};

}
}