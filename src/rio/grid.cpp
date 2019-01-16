#include "grid.hpp"
#include "infra/exception.h"
#include "infra/log.h"
#include "infra/string.h"
#include "infra/xmldocument.h"
#include "parser.hpp"
#include "strfun.hpp"
#include "xmltools.hpp"

#include <cstring>

namespace rio {

using namespace inf;

grid::grid(const std::string& name, const inf::XmlNode& cnf)
: _cellsize(0.0)
{
    if (!cnf) {
        throw std::runtime_error("Invalid TiXmlElement pointer in grid()...");
    }

    // lookup the grid name
    auto g = rio::xml::getElementByAttribute(cnf, "grid", "name", name);
    if (!g) {
        throw RuntimeError("Cannot find requested grid {}", name);
    }

    auto cellSizeAttr = str::to_numeric<double>(g.attribute("cellsize"));
    if (!cellSizeAttr.has_value()) {
        throw std::runtime_error("no/error in cellsize attribute for grid...");
    }

    _cellsize = cellSizeAttr.value();

    std::string gridfile(g.value());
    if (gridfile.empty()) {
        throw std::runtime_error("unable to get grid file name from <grid> tag");
    }
    rio::parser::get()->process(gridfile);

    _load_file(gridfile);
}

const griddefinition& grid::definition() const
{
    return _definition;
}

void grid::set_definition(griddefinition def)
{
    _definition = std::move(def);
}

void grid::_load_file(std::string filename)
{
    _cells.clear();

    Log::debug("Importing {}", filename);
    FILE* fp = fopen(filename.c_str(), "r");
    if (!fp) throw std::runtime_error("Unable to open file: " + filename);

    char line[rio::strfun::LINESIZE];
    char* ptok;

    std::vector<double> proxy;

    // read header
    fgets(line, sizeof(line), fp);
    // read file
    while (fgets(line, sizeof(line), fp)) {
        if (rio::strfun::trim(line)[0] == '#') continue;
        if (!strlen(rio::strfun::trim(line))) continue;

        if (!(ptok = strtok(line, rio::strfun::SEPCHAR))) continue;
        int id = atoi(ptok);
        if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
        double x = atof(ptok);
        if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
        double y = atof(ptok);
        if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
        double alt = atof(ptok);

        // now read the proxy field...
        proxy.clear();
        while (ptok = strtok(NULL, rio::strfun::SEPCHAR))
            proxy.push_back(atof(ptok));

        rio::cell c(id, x, y, _cellsize, alt);
        c.setProxy(proxy);

        // add the gridcell
        _cells.push_back(c);
    }

    fclose(fp);

    return;
}

grid::~grid()
{
}

std::ostream& operator<<(std::ostream& out, const grid& g)
{
    out << "grid:" << std::endl;
    for (const auto& c : g._cells)
        out << c;
    return out;
}

}
