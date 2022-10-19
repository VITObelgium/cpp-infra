#include "infra/gdalspatialreference.h"

#include "infra/exception.h"
#include "infra/gdal-private.h"
#include "infra/string.h"

namespace inf::gdal {

using namespace std::string_literals;

SpatialReference::SpatialReference()
: _srs(new OGRSpatialReference())
{
#if GDAL_VERSION_MAJOR >= 3
    _srs->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif
}

SpatialReference::SpatialReference(int32_t epsg)
: SpatialReference()
{
    import_from_epsg(epsg);
}

SpatialReference::SpatialReference(const char* wkt)
: SpatialReference()
{
    import_from_wkt(wkt);
}

SpatialReference::SpatialReference(const std::string& wkt)
: SpatialReference(wkt.c_str())
{
}

SpatialReference::SpatialReference(SpatialReference&& other) noexcept
: _srs(other._srs)
{
    other._srs = nullptr;
}

SpatialReference::SpatialReference(OGRSpatialReference* instance)
: _srs(instance)
{
    _srs->Reference();
#if GDAL_VERSION_MAJOR >= 3
    _srs->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif
}

SpatialReference::~SpatialReference() noexcept
{
    if (_srs) {
        // This does not actually delete if the reference count is still higher than 1
        _srs->Release();
    }
}

SpatialReference& SpatialReference::operator=(SpatialReference&& other)
{
    _srs       = other._srs;
    other._srs = nullptr;
    return *this;
}

SpatialReference SpatialReference::clone() const
{
    return SpatialReference(_srs->Clone());
}

SpatialReference SpatialReference::clone_geo_cs() const
{
    return SpatialReference(_srs->CloneGeogCS());
}

void SpatialReference::import_from_epsg(int32_t epsg)
{
    check_error(_srs->importFromEPSG(epsg), "Failed to import spatial reference from epsg");
}

void SpatialReference::import_from_wkt(const char* wkt)
{
    check_error(_srs->importFromWkt(wkt), "Failed to import spatial reference from WKT");
}

std::string SpatialReference::export_to_pretty_wkt() const
{
    CplPointer<char> friendlyWkt;
    check_error(_srs->exportToPrettyWkt(friendlyWkt.ptrAddress(), FALSE), "Failed to export projection to pretty WKT");
    return std::string(friendlyWkt);
}

std::string SpatialReference::export_to_wkt() const
{
    CplPointer<char> wkt;
    check_error(_srs->exportToWkt(wkt.ptrAddress()), "Failed to export projection to WKT");
    return std::string(wkt);
}

std::string SpatialReference::export_to_pretty_wkt_simplified() const
{
    CplPointer<char> friendlyWkt;
    check_error(_srs->exportToPrettyWkt(friendlyWkt.ptrAddress(), TRUE), "Failed to export projection to simplified pretty WKT");
    return std::string(friendlyWkt);
}

std::optional<int32_t> SpatialReference::epsg_cs() const noexcept
{
    if (auto code = authority_code("PROJCS"); !code.empty()) {
        return str::to_int32(code);
    } else if (auto code = authority_code("LOCAL_CS"); !code.empty()) {
        return str::to_int32(code);
    } else if (auto code = authority_code("GEOGCS"); !code.empty()) {
        return str::to_int32(code);
    }

    return {};
}

std::optional<int32_t> SpatialReference::epsg_geog_cs() const noexcept
{
    auto epsg = _srs->GetEPSGGeogCS();
    if (epsg == -1) {
        return {};
    }

    return epsg;
}

std::string_view SpatialReference::authority_code(const char* key) const noexcept
{
    auto* code = _srs->GetAuthorityCode(key);
    if (code == nullptr) {
        return {};
    }

    return std::string_view(code);
}

void SpatialReference::set_proj_cs(const char* projCs)
{
    _srs->SetProjCS(projCs);
}

void SpatialReference::set_well_known_geog_cs(const char* geogCs)
{
    _srs->SetWellKnownGeogCS(geogCs);
}

void SpatialReference::set_utm(int zone, bool north)
{
    _srs->SetUTM(zone, north ? TRUE : FALSE);
}

bool SpatialReference::is_geographic() const
{
    return _srs->IsGeographic();
}

bool SpatialReference::is_derived_geographic() const
{
    return _srs->IsDerivedGeographic();
}

bool SpatialReference::is_geocentric() const
{
    return _srs->IsGeocentric();
}

bool SpatialReference::is_projected() const
{
    return _srs->IsProjected();
}

bool SpatialReference::is_local() const
{
    return _srs->IsLocal();
}

bool SpatialReference::is_valid() const
{
    return _srs->Validate() == OGRERR_NONE;
}

OGRSpatialReference* SpatialReference::get() noexcept
{
    return _srs;
}

const OGRSpatialReference* SpatialReference::get() const noexcept
{
    return _srs;
}

CoordinateTransformer::CoordinateTransformer(SpatialReference source, SpatialReference dest)
: _sourceSRS(std::move(source))
, _targetSRS(std::move(dest))
{
    _transformer.reset(OGRCreateCoordinateTransformation(_sourceSRS.get(), _targetSRS.get()));
    if (!_transformer) {
        throw RuntimeError("Failed to create transformation");
    }
}

CoordinateTransformer::CoordinateTransformer(int32_t sourceEpsg, int32_t destEpsg)
{
    _sourceSRS.import_from_epsg(sourceEpsg);
    _targetSRS.import_from_epsg(destEpsg);

    _transformer.reset(OGRCreateCoordinateTransformation(_sourceSRS.get(), _targetSRS.get()));
    if (!_transformer) {
        throw RuntimeError("Failed to create transformation from EPSG:{} to EPSG:{}", sourceEpsg, destEpsg);
    }
}

Point<double> CoordinateTransformer::transform(const Point<double>& point) const
{
    Point<double> result = point;
    if (!_transformer->Transform(1, &result.x, &result.y)) {
        throw RuntimeError("Failed to transform point ({}, {})", point.x, point.y);
    }

    return result;
}

void CoordinateTransformer::transform_in_place(Point<double>& point) const
{
    if (!_transformer->Transform(1, &point.x, &point.y)) {
        throw RuntimeError("Failed to perform transformation");
    }
}

Coordinate CoordinateTransformer::transform(const Coordinate& coord) const
{
    Coordinate result = coord;
    if (!_transformer->Transform(1, &result.longitude, &result.latitude)) {
        throw RuntimeError("Failed to transform coordinate {}", coord);
    }

    return result;
}

void CoordinateTransformer::transform_in_place(Coordinate& coord) const
{
    if (!_transformer->Transform(1, &coord.longitude, &coord.latitude)) {
        throw RuntimeError("Failed to perform transformation");
    }
}

std::string CoordinateTransformer::source_projection() const
{
    return _sourceSRS.export_to_wkt();
}

std::string CoordinateTransformer::target_projection() const
{
    return _targetSRS.export_to_wkt();
}

OGRCoordinateTransformation* CoordinateTransformer::get()
{
    return _transformer.get();
}

Point<double> convert_point_projected(int32_t sourceEpsg, int32_t destEpsg, Point<double> point)
{
    CoordinateTransformer transformer(sourceEpsg, destEpsg);
    return transformer.transform(point);
}

Point<double> projected_to_geographic(int32_t epsg, Point<double> point)
{
    SpatialReference utm(epsg);
    utm.set_proj_cs("UTM 17 / WGS84");
    utm.set_well_known_geog_cs("WGS84");
    utm.set_utm(17);

    auto poLatLong = utm.clone_geo_cs();
    auto trans     = check_pointer(OGRCreateCoordinateTransformation(utm.get(), poLatLong.get()), "Failed to create transformation");

    if (!trans->Transform(1, &point.x, &point.y)) {
        throw RuntimeError("Failed to perform transformation");
    }

    return point;
}

std::string projection_to_friendly_name(const std::string& projection)
{
    SpatialReference spatialRef;
    spatialRef.import_from_wkt(projection.c_str());
    return spatialRef.export_to_pretty_wkt_simplified();
}

std::string projection_from_epsg(int32_t epsg)
{
    SpatialReference spatialRef(epsg);
    return spatialRef.export_to_pretty_wkt();
}

std::optional<int32_t> projection_to_geo_epsg(const std::string& projection) noexcept
{
    SpatialReference spatialRef(projection.c_str());
    return spatialRef.epsg_geog_cs();
}

std::optional<int32_t> projection_to_epsg(const std::string& projection) noexcept
{
    SpatialReference spatialRef(projection.c_str());
    return spatialRef.epsg_cs();
}

}
