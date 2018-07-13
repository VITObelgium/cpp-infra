#pragma once

#include "infra/filesystem.h"
#include "uiinfra/qstringhash.h"

#include <qgeopath.h>
#include <unordered_map>
#include <vector>

namespace infra::gdal {
class VectorDataSet;
class CoordinateTransformer;
}

namespace uiinfra {

using OverlayMap = std::unordered_map<QString, std::vector<QGeoPath>>;

OverlayMap loadShapes(const std::vector<std::pair<std::string, fs::path>>& shapes);
std::vector<QGeoPath> dataSetToGeoPath(infra::gdal::VectorDataSet& ds, const infra::gdal::CoordinateTransformer& transformer);

}
