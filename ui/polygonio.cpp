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

static void addLineToGeoPath(QGeoPath& path, gdal::LineCRef line)
{
    for (auto& point : line) {
        addPointToGeoPath(path, point);
    }
}

static void addPolyToGeoPath(std::vector<QGeoPath>& geoPaths, gdal::PolygonCRef poly)
{
    {
        QGeoPath path;
        for (auto& point : poly.exterior_ring()) {
            addPointToGeoPath(path, point);
        }
        if (!path.isEmpty()) {
            geoPaths.emplace_back(std::move(path));
        }
    }

    for (int i = 0; i < poly.interior_ring_count(); ++i) {
        QGeoPath path;
        for (auto& point : poly.interior_ring(i)) {
            addPointToGeoPath(path, point);
        }
        if (!path.isEmpty()) {
            geoPaths.emplace_back(std::move(path));
        }
    }
}

std::vector<QGeoPath> dataSetToGeoPath(inf::gdal::VectorDataSet& ds, inf::gdal::CoordinateTransformer& transformer)
{
    std::vector<QGeoPath> geoPaths;

    for (auto& feature : ds.layer(0)) {
        if (!feature.has_geometry()) {
            continue;
        }

        auto geometry = feature.geometry();

        geometry.transform(transformer);
        switch (geometry.type()) {
        case gdal::Geometry::Type::Line: {
            QGeoPath path;
            addLineToGeoPath(path, geometry.as<gdal::LineCRef>());
            geoPaths.emplace_back(std::move(path));
            break;
        }
        case gdal::Geometry::Type::MultiLine: {
            auto multiLine = geometry.as<gdal::MultiLineCRef>();
            for (int i = 0; i < multiLine.size(); ++i) {
                QGeoPath path;
                addLineToGeoPath(path, multiLine.line_at(i));
                geoPaths.emplace_back(std::move(path));
            }
            break;
        }
        case gdal::Geometry::Type::Polygon: {
            auto poly = geometry.as<gdal::PolygonCRef>();
            addPolyToGeoPath(geoPaths, poly);
            break;
        }
        case gdal::Geometry::Type::MultiPolygon: {
            auto multiPoly = geometry.as<gdal::MultiPolygonCRef>();
            for (int i = 0; i < multiPoly.size(); ++i) {
                auto poly = multiPoly.polygon_at(i);
                addPolyToGeoPath(geoPaths, poly);
            }
            break;
        }
        case gdal::Geometry::Type::Point:
            break;
        default:
            Log::warn("Unhandled type: {}", geometry.type_name());
        }
    }

    return geoPaths;
}

std::vector<QGeoPath> loadShape(const fs::path& shapePath, int32_t epsg)
{
    gdal::CoordinateTransformer transformer(epsg, 4326);
    auto ds = gdal::VectorDataSet::open(shapePath, gdal::VectorType::Unknown);
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
    auto ds = gdal::VectorDataSet::open(shapePath, gdal::VectorType::Unknown);
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

OverlayMap loadShapes(const std::vector<std::pair<std::string, fs::path>>& shapes, int32_t epsg)
{
    Log::debug("Load overlays");

    OverlayMap data;

    gdal::CoordinateTransformer transformer(epsg, 4326);
    for (auto& [shpName, shpPath] : shapes) {
        Log::debug("Load shape {} from {}", shpName, shpPath);
        auto ds = gdal::VectorDataSet::open(shpPath, gdal::VectorType::Unknown);
        if (ds.layer_count() == 0) {
            continue;
        }

        try {
            data.emplace(QString::fromStdString(shpName), dataSetToGeoPath(ds, transformer));
        } catch (const std::exception& e) {
            Log::warn("Failed to add overlay {} ({})", shpPath.u8string(), e.what());
        }
    }

    Log::debug("Overlays loaded");

    return data;
}

}
