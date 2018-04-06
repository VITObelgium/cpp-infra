#include "uiinfra/polygonio.h"

#include "infra/exception.h"
#include "infra/gdal.h"
#include "infra/gdalalgo.h"
#include "infra/log.h"

namespace uiinfra {

using namespace infra;

static void addPointToGeoPath(const gdal::CoordinateTransformer& transformer, QGeoPath& path, const infra::Point<double>& point)
{
    auto converted = transformer.transform(point);
    path.addCoordinate(QGeoCoordinate(converted.y, converted.x));
}

static void addLineToGeoPath(const gdal::CoordinateTransformer& transformer, QGeoPath& path, const gdal::Line& line)
{
    for (auto& point : line) {
        addPointToGeoPath(transformer, path, point);
    }
}

static void addPolyToGeoPath(const gdal::CoordinateTransformer& transformer, QGeoPath& path, gdal::Polygon& poly)
{
    auto ring = poly.exteriorRing();
    for (auto& point : ring) {
        addPointToGeoPath(transformer, path, point);
    }
}

static std::vector<QGeoPath> dataSetToGeoPath(gdal::VectorDataSet& ds, const gdal::CoordinateTransformer& transformer)
{
    std::vector<QGeoPath> geoPaths;

    for (auto& feature : ds.getLayer(0)) {
        auto geometry = feature.geometry();
        switch (geometry.type()) {
        case gdal::Geometry::Type::Line: {
            QGeoPath path;
            addLineToGeoPath(transformer, path, geometry.as<gdal::Line>());
            geoPaths.push_back(path);
            break;
        }
        case gdal::Geometry::Type::MultiLine: {
            QGeoPath path;
            auto multiLine = geometry.as<gdal::MultiLine>();
            for (int i = 0; i < multiLine.size(); ++i) {
                addLineToGeoPath(transformer, path, multiLine.lineAt(i));
            }
            geoPaths.push_back(path);
            break;
        }
        case gdal::Geometry::Type::Polygon: {
            QGeoPath path;
            auto poly = geometry.as<gdal::Polygon>();
            addPolyToGeoPath(transformer, path, poly);
            geoPaths.push_back(path);
            break;
        }
        case gdal::Geometry::Type::MultiPolygon: {
            auto multiPoly = geometry.as<gdal::MultiPolygon>();
            for (int i = 0; i < multiPoly.size(); ++i) {
                QGeoPath path;
                auto poly = multiPoly.polygonAt(i);
                addPolyToGeoPath(transformer, path, poly);
                geoPaths.push_back(path);
            }
            break;
        }
        default:
            Log::warn("Unhandled type: {}", geometry.typeName());
        }
    }

    return geoPaths;
}

OverlayMap loadShapes(const std::vector<std::pair<std::string, fs::path>>& shapes)
{
    Log::debug("Load overlays");

    OverlayMap data;

    gdal::CoordinateTransformer transformer1(31370, 4326);
    for (auto& [shpName, shpPath] : shapes) {
        auto ds = gdal::VectorDataSet::create(shpPath, gdal::VectorType::Unknown);
        if (ds.layerCount() == 0) {
            continue;
        }

        try {
            data.emplace(shpName, dataSetToGeoPath(ds, transformer1));
        } catch (const std::exception& e) {
            Log::warn("Failed to add overlay {} ({})", shpPath.string(), e.what());
        }
    }

    Log::debug("Overlays loaded");

    return data;
}
}
