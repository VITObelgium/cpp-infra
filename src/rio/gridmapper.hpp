#pragma once

#include <cstdio>
#include <memory>
#include <string>

#include "infra/cell.h"
#include "infra/filesystem.h"
#include "infra/geometadata.h"
#include "outputhandler.hpp"

namespace rio {

struct griddefinition
{
    inf::GeoMetadata metadata;
    std::string mapfilePattern;
};

std::unordered_map<int64_t, inf::Cell> read_mapping_file(const fs::path& path);

}
