#pragma once

#include "infra/filesystem.h"

#include <qgeopath.h>
#include <unordered_map>
#include <vector>

namespace uiinfra {

using OverlayMap = std::unordered_map<std::string, std::vector<QGeoPath>>;

OverlayMap loadShapes(const std::vector<std::pair<std::string, fs::path>>& shapes);
}
