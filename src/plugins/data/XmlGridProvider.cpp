/*
 * XmlGridProvider.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "XmlGridProvider.h"
#include "Exceptions.h"
#include "infra/configdocument.h"

namespace opaq {

using namespace infra;

std::string XmlGridProvider::name()
{
    return "xmlgridprovider";
}

/**
 * configuration example:
 * 	zmin and zmax are optional
 * <grid>
 * 	<cell id="1" xmin="12.1" xmax="14.1" ymin="10.1" ymax="11.1" zmin="0" zmax="1"/>
 * 	<cell id="1" xmin="12.1" xmax="14.1" ymin="11.1" ymax="12.1"/>
 * </grid>
 */
void XmlGridProvider::configure(const ConfigNode& configuration, const std::string& componentName, IEngine&)
{
    setName(componentName);

    auto gridEl = configuration.child("grid");
    if (!gridEl) {
        throw BadConfigurationException("grid element not found in configuration");
    }

    std::vector<int> cellIds;
    for (auto& cellEl : gridEl.children("cell")) {
        auto id   = cellEl.attribute<int>("id");
        auto xmin = cellEl.attribute<double>("xmin");
        auto xmax = cellEl.attribute<double>("xmax");
        auto ymin = cellEl.attribute<double>("ymin");
        auto ymax = cellEl.attribute<double>("ymax");
        auto zmin = cellEl.attribute<double>("zmin");
        auto zmax = cellEl.attribute<double>("zmax");

        if (!id || !xmin || !xmax || !ymin || !ymax) {
            throw BadConfigurationException("cell should at least have id, xmin, xmax, ymin, and ymax defined");
        }

        // check if id is unique
        if (std::find(cellIds.begin(), cellIds.end(), id) != cellIds.end()) {
            throw BadConfigurationException("Duplicate cell id: {}", *id);
        }

        cellIds.push_back(id.value());

        // create cell and push into the grid
        _grid.addCell(Cell(*id, *xmin, *xmax, *ymin, *ymax, zmin.value_or(0), zmax.value_or(0)));
    }

    if (_grid.cellCount() == 0) {
        throw BadConfigurationException("no cells defined in grid");
    }
}

const Grid& XmlGridProvider::getGrid(const std::string&, GridType)
{
    return _grid;
}

}
