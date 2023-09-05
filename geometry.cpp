#include "infra/geometry.h"

#include "infra/algo.h"
#include "infra/chrono.h"
#include "infra/exception.h"
#include "infra/log.h"

#include "infra/gdalgeometry.h"

#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/version.h>

#if GEOS_VERSION_MAJOR > 3 || (GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR >= 10)
#define HAVE_GEOJSON_EXPORT 1
#endif

#ifdef HAVE_GEOJSON_EXPORT
#include <geos/io/GeoJSONWriter.h>
#endif

#include <cassert>
#include <memory>
#include <vector>

namespace inf::geom {

using namespace inf;

std::unique_ptr<geos::geom::LinearRing> gdal_linear_ring_to_geos(const geos::geom::GeometryFactory& factory, gdal::LinearRingCRef ring)
{
    auto coords = std::make_unique<geos::geom::CoordinateSequence>(geos::geom::CoordinateSequence::XY(ring.point_count()));

    for (int i = 0; i < ring.point_count(); ++i) {
        const auto point = ring.point_at(i);
        coords->setAt(geos::geom::Coordinate(point.x, point.y), i);
    }

    return factory.createLinearRing(std::move(coords));
}

static geos::geom::Polygon::Ptr gdal_polygon_to_geos(const geos::geom::GeometryFactory& factory, gdal::PolygonCRef poly)
{
    std::unique_ptr<geos::geom::LinearRing> exteriorRing;
    std::vector<std::unique_ptr<geos::geom::LinearRing>> holes;

    {
        // Exterior ring
        exteriorRing = gdal_linear_ring_to_geos(factory, poly.exterior_ring());
    }

    // interior rings
    for (int i = 0; i < poly.interior_ring_count(); ++i) {
        holes.push_back(gdal_linear_ring_to_geos(factory, poly.interior_ring(i)));
    }

    return factory.createPolygon(std::move(exteriorRing), std::move(holes));
}

static geos::geom::MultiPolygon::Ptr gdal_multi_polygon_to_geos(const geos::geom::GeometryFactory& factory, gdal::MultiPolygonCRef geom)
{
    std::vector<std::unique_ptr<geos::geom::Geometry>> geometries;

    for (int i = 0; i < geom.size(); ++i) {
        geometries.push_back(gdal_polygon_to_geos(factory, geom.polygon_at(i)));
    }

    return factory.createMultiPolygon(std::move(geometries));
}

geos::geom::MultiPolygon::Ptr gdal_to_geos(inf::gdal::GeometryCRef geom)
{
    auto factory = geos::geom::GeometryFactory::create();

    if (geom.type() == gdal::Geometry::Type::Polygon) {
        std::vector<std::unique_ptr<geos::geom::Geometry>> geometries;
        geometries.push_back(gdal_polygon_to_geos(*factory, geom.as<gdal::PolygonCRef>()));
        return factory->createMultiPolygon(std::move(geometries));
    } else if (geom.type() == gdal::Geometry::Type::MultiPolygon) {
        return gdal_multi_polygon_to_geos(*factory, geom.as<gdal::MultiPolygonCRef>());
    }

    throw RuntimeError("Geometry type not implemented");
}

static std::unique_ptr<geos::geom::LinearRing> create_linear_ring(const geos::geom::GeometryFactory& factory, inf::Point<double> p1, inf::Point<double> p2)
{
    return factory.createLinearRing(geos::geom::CoordinateSequence({
        geos::geom::Coordinate(p1.x, p1.y),
        geos::geom::Coordinate(p2.x, p1.y),
        geos::geom::Coordinate(p2.x, p2.y),
        geos::geom::Coordinate(p1.x, p2.y),
        geos::geom::Coordinate(p1.x, p1.y),
    }));
}

geos::geom::Polygon::Ptr create_polygon(inf::Point<double> p1, inf::Point<double> p2)
{
    const auto factory = geos::geom::GeometryFactory::getDefaultInstance();
    return factory->createPolygon(create_linear_ring(*factory, p1, p2));
}

geos::geom::LinearRing::Ptr create_linear_ring_from_rect(inf::Point<double> p1, inf::Point<double> p2)
{
    const auto factory = geos::geom::GeometryFactory::getDefaultInstance();
    return create_linear_ring(*factory, p1, p2);
}

std::string to_geojson(const geos::geom::Geometry& geom)
{
#ifdef HAVE_GEOJSON_EXPORT
    geos::io::GeoJSONWriter writer;
    return writer.write(&geom);
#endif

    (void)geom;
    throw RuntimeError("Export to geojson not supported");
}

std::string to_geojson(const geos::geom::Envelope& env)
{
    auto geomFactory = geos::geom::GeometryFactory::create();
    geos::geom::CoordinateSequence coords({
        geos::geom::Coordinate(env.getMinX(), env.getMaxY()),
        geos::geom::Coordinate(env.getMaxX(), env.getMaxY()),
        geos::geom::Coordinate(env.getMaxX(), env.getMinY()),
        geos::geom::Coordinate(env.getMinX(), env.getMinY()),
        geos::geom::Coordinate(env.getMinX(), env.getMaxY()),
    });

    const auto poly = geomFactory->createPolygon(geomFactory->createLinearRing(coords), {});
    return to_geojson(*poly);
}

std::string to_geojson(const inf::GeoMetadata& meta)
{
    auto topLeft     = meta.top_left();
    auto bottomRight = meta.bottom_right();

    auto geomFactory = geos::geom::GeometryFactory::create();
    geos::geom::CoordinateSequence coords({
        geos::geom::Coordinate(topLeft.x, topLeft.y),
        geos::geom::Coordinate(bottomRight.x, topLeft.y),
        geos::geom::Coordinate(bottomRight.x, bottomRight.y),
        geos::geom::Coordinate(topLeft.x, bottomRight.y),
        geos::geom::Coordinate(topLeft.x, topLeft.y),
    });

    const auto poly = geomFactory->createPolygon(geomFactory->createLinearRing(coords), {});
    return to_geojson(*poly);
}

void calculate_geometry_envelopes(const geos::geom::Geometry& geom)
{
    const auto numGeometries = geom.getNumGeometries();

    if (numGeometries == 1) {
        geom.getEnvelope();
    } else {
        for (size_t i = 0; i < numGeometries; ++i) {
            geom.getGeometryN(i)->getEnvelope();
        }
    }
}

CoordinateWarpFilter::CoordinateWarpFilter(int32_t sourceEpsg, int32_t destEpsg)
: _transformer(sourceEpsg, destEpsg)
{
}

CoordinateWarpFilter::CoordinateWarpFilter(const char* sourceProjection, const char* destProjection)
: _transformer(gdal::SpatialReference(sourceProjection), gdal::SpatialReference(destProjection))
{
}

void CoordinateWarpFilter::filter_rw(geos::geom::CoordinateSequence& seq, std::size_t i)
{
    auto coordinate = seq.getAt(i);
    Point p(coordinate.x, coordinate.y);
    _transformer.transform_in_place(p);
    seq.setAt(geos::geom::Coordinate(p.x, p.y), i);
}

void CoordinateWarpFilter::filter_ro(const geos::geom::CoordinateSequence& /*seq*/, std::size_t /*i*/)
{
    assert(false);
}

bool CoordinateWarpFilter::isDone() const
{
    return false;
}

bool CoordinateWarpFilter::isGeometryChanged() const
{
    return true;
}
}
