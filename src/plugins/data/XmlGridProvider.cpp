/*
 * XmlGridProvider.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "XmlGridProvider.h"
#include "Exceptions.h"
#include "ComponentManager.h"

#include <tinyxml.h>

namespace OPAQ {

XmlGridProvider::XmlGridProvider()
: _logger("OPAQ::XmlGridProvider") {
}

XmlGridProvider::~XmlGridProvider() {
	std::vector<Cell *> * cells = &(_grid.getCells());
	std::vector<Cell *>::iterator it = cells->begin();
	while (it != cells->end()) {
		Cell * toErase = *it++;
		delete toErase;
	}
}

/**
 * configuration example:
 * 	zmin and zmax are optional
 * <grid>
 * 	<cell id="1" xmin="12.1" xmax="14.1" ymin="10.1" ymax="11.1" zmin="0" zmax="1"/>
 * 	<cell id="1" xmin="12.1" xmax="14.1" ymin="11.1" ymax="12.1"/>
 * </grid>
 */
void XmlGridProvider::configure(TiXmlElement * configuration, const std::string& componentName, IEngine&) {
	setName(componentName);

	TiXmlElement * gridEl = configuration->FirstChildElement("grid");
	if (!gridEl) {
		_logger->error("grid element not found in configuration");
		throw BadConfigurationException("grid element not found in configuration");
	}

	std::vector<int> cellIds;

	TiXmlElement * cellEl = gridEl->FirstChildElement("cell");
	while (cellEl) {
		int id;
		double xmin, xmax, ymin, ymax, zmin, zmax;

		if (cellEl->QueryIntAttribute("id", &id) != TIXML_SUCCESS
				|| cellEl->QueryDoubleAttribute("xmin", &xmin) != TIXML_SUCCESS
				|| cellEl->QueryDoubleAttribute("xmax", &xmax) != TIXML_SUCCESS
				|| cellEl->QueryDoubleAttribute("ymin", &ymin) != TIXML_SUCCESS
				|| cellEl->QueryDoubleAttribute("ymax", &ymax) != TIXML_SUCCESS) {
			throw BadConfigurationException("cell should at least have id, xmin, xmax, ymin, and ymax defined");
		}

		// zmin and zmax are optional, default is 0
		if (cellEl->QueryDoubleAttribute("zmin", &zmin) != TIXML_SUCCESS)
			zmin = 0;
		if (cellEl->QueryDoubleAttribute("zmax", &zmax) != TIXML_SUCCESS)
			zmax = 0;

		// check if id is unique
		if (std::find(cellIds.begin(), cellIds.end(), id) != cellIds.end()) {
			std::stringstream ss;
			ss << "duplicate cell id: " << id;
			throw BadConfigurationException(ss.str());
		} else {
			cellIds.push_back(id);
		}

		// create cell and push into the grid
		Cell * cell = new Cell(id, xmin, xmax, ymin, ymax, zmin, zmax);
		_grid.getCells().push_back(cell);


		cellEl = cellEl->NextSiblingElement("cell");
	}

	if (_grid.getCells().size() == 0)
		throw BadConfigurationException("no cells defined in grid");
}

} /* namespace OPAQ */

OPAQ_REGISTER_PLUGIN(OPAQ::XmlGridProvider)
