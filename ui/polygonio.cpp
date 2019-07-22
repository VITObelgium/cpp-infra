#include "uiinfra/polygonio.h"

#include "infra/exception.h"
#include "infra/gdal.h"
#include "infra/gdalalgo.h"
#include "infra/log.h"

#include <qvector.h>

namespace inf::ui {

using namespace inf;

static void addPointToGeoPath(QGeoPath& path, const inf::Point<double>& point)
{
    path.addCoordinate(QGeoCoordinate(point.y, point.x));
}

static void addLineToGeoPath(QGeoPath& path, const gdal::Line& line)
{
    for (auto& point : line) {
        addPointToGeoPath(path, point);
    }
}

static void addPolyToGeoPath(std::vector<GeoPathWithId>& geoPaths, gdal::Polygon& poly, int64_t id)
{
    auto ring = poly.exteriorring();
    {
        GeoPathWithId pathWithId;
        pathWithId.id = id;
        for (auto& point : ring) {
            addPointToGeoPath(pathWithId.path, point);
        }
        if (!pathWithId.path.isEmpty()) {
            geoPaths.emplace_back(std::move(pathWithId));
        }
    }

    for (int i = 0; i < poly.interiorring_count(); ++i) {
        GeoPathWithId pathWithId;
        pathWithId.id = id;
        ring = poly.interiorring(i);
        for (auto& point : ring) {
            addPointToGeoPath(pathWithId.path, point);
        }
        if (!pathWithId.path.isEmpty()) {
            geoPaths.emplace_back(std::move(pathWithId));
        }
    }
}

std::vector<GeoPathWithId> dataSetToGeoPathWithInt64Id(inf::gdal::VectorDataSet& ds, inf::gdal::CoordinateTransformer& transformer, const std::string& fieldNameContainingId)
{
    std::vector<GeoPathWithId> geoPaths;

    auto idIndex = fieldNameContainingId.size() ? ds.layer(0).field_index(fieldNameContainingId) : (int64_t) 0;

    for (auto& feature : ds.layer(0)) {
        if (!feature.has_geometry()) {
            continue;
        }

        auto geometry = feature.geometry();
        auto id = fieldNameContainingId.size() ? feature.field_as<int64_t>(idIndex) :  (int64_t) 0;

        geometry.transform(transformer);
        switch (geometry.type()) {
        case gdal::Geometry::Type::Line: {
            GeoPathWithId pathWithId;
            addLineToGeoPath(pathWithId.path, geometry.as<gdal::Line>());
            pathWithId.id = id;
            geoPaths.push_back(pathWithId);
            break;
        }
        case gdal::Geometry::Type::MultiLine: {
            auto multiLine = geometry.as<gdal::MultiLine>();
            for (int i = 0; i < multiLine.size(); ++i) {
                GeoPathWithId pathWithId;
                addLineToGeoPath(pathWithId.path, multiLine.line_at(i));
                pathWithId.id = id;
                geoPaths.push_back(pathWithId);
            }
            break;
        }
        case gdal::Geometry::Type::Polygon: {
            auto poly = geometry.as<gdal::Polygon>();
            addPolyToGeoPath(geoPaths, poly, id);
            break;
        }
        case gdal::Geometry::Type::MultiPolygon: {
            auto multiPoly = geometry.as<gdal::MultiPolygon>();
            for (int i = 0; i < multiPoly.size(); ++i) {
                auto poly = multiPoly.polygon_at(i);
                addPolyToGeoPath(geoPaths, poly, id);
            }
            break;
        }
        default:
            Log::warn("Unhandled type: {}", geometry.type_name());
        }
    }

    return geoPaths;
}

std::vector<QGeoPath> dataSetToGeoPath(gdal::VectorDataSet& ds, gdal::CoordinateTransformer& transformer)
{
    auto geoPathsWithId = dataSetToGeoPathWithInt64Id(ds, transformer, "");
    std::vector<QGeoPath> geoPaths;
    for(const auto& geoPathWithId : geoPathsWithId)
        geoPaths.push_back(geoPathWithId.path);
    return geoPaths;
}

std::vector<QGeoPath> loadShape(const fs::path& shapePath, int32_t epsg)
{
    gdal::CoordinateTransformer transformer(epsg, 4326);
    auto ds = gdal::VectorDataSet::create(shapePath, gdal::VectorType::Unknown);
    if (ds.layer_count() > 0) {
        try {
            return dataSetToGeoPath(ds, transformer);
        } catch (const std::exception& e) {
            Log::warn("Failed to add overlay {} ({})", shapePath, e.what());
        }
    }

    return {};
}

std::vector<QGeoPath> loadShape(const fs::path& shapePath, int32_t epsg, inf::Rect<double>& extent)
{
    gdal::CoordinateTransformer transformer(epsg, 4326);
    auto ds = gdal::VectorDataSet::create(shapePath, gdal::VectorType::Unknown);
    if (ds.layer_count() > 0) {
        try {
            extent = ds.layer(0).extent();
            transformer.transform_in_place(extent.topLeft);
            transformer.transform_in_place(extent.bottomRight);
            return dataSetToGeoPath(ds, transformer);
        } catch (const std::exception& e) {
            Log::warn("Failed to add overlay {} ({})", shapePath, e.what());
        }
    }

    return {};
}

OverlayMapWithId loadShapes(const std::vector<std::pair<std::string, fs::path>>& shapes, int32_t epsg, const std::string& fieldNameContainingId)
{
    Log::debug("Load overlays");

    OverlayMapWithId data;

    gdal::CoordinateTransformer transformer(epsg, 4326);
    for (auto& [shpName, shpPath] : shapes) {
        auto ds = gdal::VectorDataSet::create(shpPath, gdal::VectorType::Unknown);
        if (ds.layer_count() == 0) {
            continue;
        }

        try {
            data.emplace(QString::fromStdString(shpName), dataSetToGeoPathWithInt64Id(ds, transformer, fieldNameContainingId));
        } catch (const std::exception& e) {
            Log::warn("Failed to add overlay {} ({})", shpPath.u8string(), e.what());
        }
    }

    Log::debug("Overlays loaded");

    return data;
}

OverlayMap loadShapes(const std::vector<std::pair<std::string, fs::path>>& shapes, int32_t epsg)
{
    auto overlayMapWithId = loadShapes(shapes, epsg, "");
    OverlayMap overlayMap;
    for(const auto& item : overlayMapWithId) {
        std::vector<QGeoPath> geoPaths;
        for(const auto& geoPath : item.second) {
            geoPaths.push_back(geoPath.path);
        }
        overlayMap.emplace(item.first, geoPaths);
    }
    return overlayMap;
}
}
