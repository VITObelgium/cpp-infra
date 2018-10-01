#pragma once

#include "infra/filesystem.h"
#include "uiinfra/qstringhash.h"

#include <qgeopath.h>
#include <unordered_map>
#include <vector>

namespace inf::gdal {
class VectorDataSet;
class CoordinateTransformer;
}

namespace uiinfra {

using OverlayMap = std::unordered_map<QString, std::vector<QGeoPath>>;

OverlayMap loadShapes(const std::vector<std::pair<std::string, fs::path>>& shapes, int32_t epsg);
std::vector<QGeoPath> dataSetToGeoPath(inf::gdal::VectorDataSet& ds, const inf::gdal::CoordinateTransformer& transformer);

}
