#pragma once

#include "infra/gdal.h"
#include "infra/gdalgeometry.h"
#include "infra/gdalspatialreference.h"
#include "infra/geometadata.h"
#include "infra/point.h"

#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/MultiPolygon.h>

namespace inf::geom {

geos::geom::MultiPolygon::Ptr gdal_to_geos(inf::gdal::GeometryCRef geom);
geos::geom::Polygon::Ptr create_polygon(inf::Point<double> p1, inf::Point<double> p2);
geos::geom::LinearRing::Ptr create_linear_ring_from_rect(inf::Point<double> p1, inf::Point<double> p2);

std::string to_geojson(const geos::geom::Geometry& geom);
std::string to_geojson(const geos::geom::Envelope& env);
std::string to_geojson(const GeoMetadata& meta);

void calculate_geometry_envelopes(const geos::geom::Geometry& geom);

class CoordinateWarpFilter : public geos::geom::CoordinateSequenceFilter
{
public:
    CoordinateWarpFilter(int32_t sourceEpsg, int32_t destEpsg);
    CoordinateWarpFilter(const char* sourceProjection, const char* destProjection);
    virtual ~CoordinateWarpFilter() = default;

    void filter_rw(geos::geom::CoordinateSequence& seq, std::size_t i) override;
    void filter_ro(const geos::geom::CoordinateSequence& seq, std::size_t i) override;

    bool isDone() const override;
    bool isGeometryChanged() const override;

private:
    gdal::CoordinateTransformer _transformer;
};

}
