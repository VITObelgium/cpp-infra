#pragma once

#include "infra/coordinate.h"
#include "infra/point.h"

#include <gdal_version.h>
#include <memory>
#include <ogr_spatialref.h>
#include <optional>
#include <string>

namespace inf::gdal {

/*! Wrapper around the OGRSpatialReference class
 * Needed as the OGRSpatialReference destructor is deprecated
 * so it is very easy to misuse the OGRSpatialReference API
 */
class SpatialReference
{
public:
    SpatialReference();
    SpatialReference(int32_t epsg);
    SpatialReference(const char* wkt);
    SpatialReference(const std::string& wkt);
    SpatialReference(OGRSpatialReference* instance);
    ~SpatialReference() noexcept;

    SpatialReference& operator=(SpatialReference&&);

    SpatialReference(SpatialReference&&) noexcept;

    SpatialReference clone() const;
    SpatialReference clone_geo_cs() const;

    void import_from_epsg(int32_t epsg);
    void import_from_wkt(const char* wkt);

    std::string export_to_wkt() const;
    std::string export_to_pretty_wkt() const;
    std::string export_to_pretty_wkt_simplified() const;

    std::optional<int32_t> epsg_cs() const noexcept;
    std::optional<int32_t> epsg_geog_cs() const noexcept;
    std::string_view authority_code(const char* key) const noexcept;

    void set_proj_cs(const char* projCs);
    void set_well_known_geog_cs(const char* geogCs);
    void set_utm(int zone, bool north = true);

    bool is_geographic() const;
#if GDAL_VERSION_MAJOR >= 3
    bool is_derived_geographic() const;
#endif

    bool is_geocentric() const;
    bool is_projected() const;
    bool is_local() const;

    bool is_valid() const;

    OGRSpatialReference* get() noexcept;
    const OGRSpatialReference* get() const noexcept;

private:
    OGRSpatialReference* _srs = nullptr;
};

class CoordinateTransformer
{
public:
    CoordinateTransformer(SpatialReference source, SpatialReference dest);
    CoordinateTransformer(int32_t sourceEpsg, int32_t destEpsg);

    Point<double> transform(const Point<double>& point) const;
    void transform_in_place(Point<double>& point) const;

    Coordinate transform(const Coordinate& coord) const;
    void transform_in_place(Coordinate& coord) const;

    std::string source_projection() const;
    std::string target_projection() const;

    OGRCoordinateTransformation* get();

private:
    SpatialReference _sourceSRS;
    SpatialReference _targetSRS;
    std::unique_ptr<OGRCoordinateTransformation> _transformer;
};

/* convenience function to convert a single point (internally creates a CoordinateTransformer)
 * Don't use this function for converting a lot of points as there is a significant overhead
 * in creating a CoordinateTransformer instance for every point
 */
Point<double> convert_point_projected(int32_t sourceEpsg, int32_t destEpsg, Point<double> point);
Point<double> projected_to_geographic(int32_t epsg, Point<double>);
std::string projection_to_friendly_name(const std::string& projection);
std::string projection_from_epsg(int32_t epsg);
std::optional<int32_t> projection_to_geo_epsg(const std::string& projection) noexcept;
std::optional<int32_t> projection_to_epsg(const std::string& projection) noexcept;

}
