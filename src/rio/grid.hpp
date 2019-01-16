#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "cell.hpp"
#include "griddefinition.hpp"

namespace inf {
class XmlNode;
}

namespace rio {

class grid
{
public:
    grid(const std::string& name, const inf::XmlNode& cnf);
    virtual ~grid();

    const std::vector<cell>& cells() const
    {
        return _cells;
    }

    size_t size() const
    {
        return _cells.size();
    }

    const griddefinition& definition() const;
    void set_definition(griddefinition def);

public:
    friend std::ostream& operator<<(std::ostream& out, const grid& g);

private:
    std::vector<cell> _cells;
    double _cellsize;

    void _load_file(std::string filename);
    griddefinition _definition;
};

}
