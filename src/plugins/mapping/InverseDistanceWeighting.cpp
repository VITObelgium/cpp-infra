
#include "InverseDistanceWeighting.h"
#include "PluginRegistration.h"
#include "data/IGridProvider.h"

#include <tinyxml.h>

namespace opaq
{

using namespace std::chrono_literals;

InverseDistanceWeighting::InverseDistanceWeighting()
{
}

std::string InverseDistanceWeighting::name()
{
    return "idwmodel";
}

void InverseDistanceWeighting::configure(TiXmlElement* cnf, const std::string& componentName, IEngine&)
{
    setName(componentName);
}

void InverseDistanceWeighting::run()
{
    // for each cell in the grid
    // calculate the iwd based on all the stations

    /*auto& grid = getGridProvider().getGrid(getPollutant().getName(), GridType::Grid4x4);
    for (size_t i = 0; i < grid.cellCount(); ++i)
    {
        auto cell = grid.cell(i);
    }*/
}

OPAQ_REGISTER_STATIC_PLUGIN(InverseDistanceWeighting)

}

