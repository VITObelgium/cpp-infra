#include <cstring>
#include <tinyxml.h>

#include "grid.hpp"
#include "parser.hpp"
#include "strfun.hpp"
#include "xmltools.hpp"

namespace rio {

grid::grid(const std::string& name, TiXmlElement* cnf)
: _cellsize(0.)
{
    if (!cnf) throw std::runtime_error("Invalid TiXmlElement pointer in grid()...");

    // lookup the grid name
    TiXmlElement* g = rio::xml::getElementByAttribute(cnf, "grid", "name", name);
    if (!g) throw std::runtime_error("Cannot find requested grid " + name);

    if (g->QueryDoubleAttribute("cellsize", &_cellsize) != TIXML_SUCCESS)
        throw std::runtime_error("no/error in cellsize attribute for grid...");

    std::string gridfile;
    try {
        gridfile = g->GetText();
    } catch (...) {
        throw std::runtime_error("unable to get grid file name from <grid> tag");
    }
    rio::parser::get()->process(gridfile);

    _load_file(gridfile);
}

void grid::_load_file(std::string filename)
{
    _cells.clear();

    std::cout << " Importing " << filename << std::endl;
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
