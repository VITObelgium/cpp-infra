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
    MappingStage(Component stations, std::vector<Component> models);

    Component getStationProvider() const;
    std::vector<Component> getModels() const;

private:
    Component _stations;
    std::vector<Component> _models;
};

}
}
