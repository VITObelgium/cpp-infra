#pragma once

#include "infra/filesystem.h"
#include "infra/rect.h"
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

std::vector<QGeoPath> loadShape(const fs::path& shapePath, int32_t epsg);
std::vector<QGeoPath> loadShape(const fs::path& shapePath, int32_t epsg, inf::Rect<double>& extent);

OverlayMap loadShapes(const std::vector<std::pair<std::string, fs::path>>& shapes, int32_t epsg);
std::vector<QGeoPath> dataSetToGeoPath(inf::gdal::VectorDataSet& ds, inf::gdal::CoordinateTransformer& transformer);

}
