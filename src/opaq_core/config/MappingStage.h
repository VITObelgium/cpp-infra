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
    MappingStage(Component stations, Component obsProvider, std::vector<Component> models);

    Component getStationProvider() const;
    Component getDataProvider() const;
    std::vector<Component> getModels() const;

private:
    Component _stations;
    Component _obsProvider;
    std::vector<Component> _models;
};

}
}
