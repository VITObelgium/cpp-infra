#include "uiinfra/polygonio.h"

#include "infra/exception.h"
#include "infra/gdal.h"
#include "infra/gdalalgo.h"
#include "infra/log.h"

#include <qvector.h>

namespace uiinfra {

using namespace inf;

static void addPointToGeoPath(const gdal::CoordinateTransformer& transformer, QGeoPath& path, const inf::Point<double>& point)
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

static void addPolyToGeoPath(const gdal::CoordinateTransformer& transformer, std::vector<QGeoPath>& geoPaths, gdal::Polygon& poly)
{
    auto ring = poly.exteriorring();
    {
        QGeoPath path;
        for (auto& point : ring) {
            addPointToGeoPath(transformer, path, point);
        }
        geoPaths.emplace_back(std::move(path));
    }

    for (int i = 0; i < poly.interiorring_count(); ++i) {
        QGeoPath path;
        ring = poly.interiorring(i);
        for (auto& point : ring) {
            addPointToGeoPath(transformer, path, point);
        }
        geoPaths.emplace_back(std::move(path));
    }
}

std::vector<QGeoPath> dataSetToGeoPath(gdal::VectorDataSet& ds, const gdal::CoordinateTransformer& transformer)
{
    std::vector<QGeoPath> geoPaths;

    for (auto& feature : ds.layer(0)) {
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
                addLineToGeoPath(transformer, path, multiLine.line_at(i));
            }
            geoPaths.push_back(path);
            break;
        }
        case gdal::Geometry::Type::Polygon: {
            auto poly = geometry.as<gdal::Polygon>();
            addPolyToGeoPath(transformer, geoPaths, poly);
            break;
        }
        case gdal::Geometry::Type::MultiPolygon: {
            auto multiPoly = geometry.as<gdal::MultiPolygon>();
            for (int i = 0; i < multiPoly.size(); ++i) {
                auto poly = multiPoly.polygon_at(i);
                addPolyToGeoPath(transformer, geoPaths, poly);
            }
            break;
        }
        default:
            Log::warn("Unhandled type: {}", geometry.type_name());
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
        if (ds.layer_count() == 0) {
            continue;
        }

        try {
            data.emplace(QString::fromUtf8(shpName.c_str()), dataSetToGeoPath(ds, transformer1));
        } catch (const std::exception& e) {
            Log::warn("Failed to add overlay {} ({})", shpPath.string(), e.what());
        }
    }

    Log::debug("Overlays loaded");

    return data;
}
}
