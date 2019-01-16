#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "cell.hpp"
#include "infra/geometadata.h"

namespace rio {

struct griddefinition
{
    std::string name;
    std::string mapfilePattern; //! mapping file specification to translate from cell ids to grind index
    inf::GeoMetadata metadata;
};

}
