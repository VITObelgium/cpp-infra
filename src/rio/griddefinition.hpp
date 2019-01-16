#pragma once

#include "infra/geometadata.h"

#include <string>

namespace rio {

struct griddefinition
{
    std::string name;
    std::string mapfilePattern; //! mapping file specification to translate from cell ids to grind index
    inf::GeoMetadata metadata;
};

}
