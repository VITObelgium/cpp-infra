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

namespace inf::ui {

struct GeoPathWithId
{
    int64_t id;
    QGeoPath path;
};

using OverlayMapWithId = std::unordered_map<QString, std::vector<GeoPathWithId>>;

std::vector<QGeoPath> loadShape(const fs::path& shapePath, int32_t epsg);
std::vector<QGeoPath> loadShape(const fs::path& shapePath, int32_t epsg, inf::Rect<double>& extent);

OverlayMapWithId loadShapes(const std::vector<std::pair<std::string, fs::path>>& shapes, int32_t epsg, const std::string& fieldNameContainingId);

std::vector<GeoPathWithId> dataSetToGeoPathWithInt64Id(inf::gdal::VectorDataSet& ds, inf::gdal::CoordinateTransformer& transformer, const std::string& fieldNameContainingId);

// Deprecated API, provided for backwards compatibility
using OverlayMap = std::unordered_map<QString, std::vector<QGeoPath>>;
OverlayMap loadShapes(const std::vector<std::pair<std::string, fs::path>>& shapes, int32_t epsg);
std::vector<QGeoPath> dataSetToGeoPath(inf::gdal::VectorDataSet& ds, inf::gdal::CoordinateTransformer& transformer);

}
