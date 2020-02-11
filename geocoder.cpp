#include "infra/geocoder.h"
#include "infra/conversion.h"
#include "infra/exception.h"
#include "infra/gdal.h"
#include "infra/gdalgeometry.h"
#include "infra/scopeguard.h"

#include <cpl_port.h>
#include <ogr_geocoding.h>
#include <ogrsf_frmts.h>

namespace inf {

std::string Geocoder::Osm      = "OSM_NOMINATIM";
std::string Geocoder::MapQuest = "MAPQUEST_NOMINATIM";
std::string Geocoder::GeoNames = "GEONAMES";
std::string Geocoder::Bing     = "BING";

struct Geocoder::Pimpl
{
    OGRGeocodingSessionH gc;
};

Geocoder::Geocoder()
: _pimpl(std::make_unique<Pimpl>())
{
    _pimpl->gc = OGRGeocodeCreateSession(nullptr);
    throw_on_invalid_handle();
}

Geocoder::Geocoder(const Options& options)
: _pimpl(std::make_unique<Pimpl>())
{
    _allowUnsafeSsl = options.allowUnsafeSsl;

    gdal::StringOptions optionsArray;
    optionsArray.add(fmt::format("SERVICE={}", options.service));

    if (!options.cacheFile.empty()) {
        optionsArray.add(fmt::format("CACHE_FILE={}", options.cacheFile));
    }

    if (!options.readCache) {
        optionsArray.add("READ_CACHE=FALSE");
    }

    if (!options.writeCache) {
        optionsArray.add("WRITE_CACHE=FALSE");
    }

    if (!options.email.empty()) {
        optionsArray.add(fmt::format("EMAIL={}", options.email));
    }

    if (!options.key.empty()) {
        optionsArray.add(fmt::format("KEY={}", options.key));
    }

    if (!options.application.empty()) {
        optionsArray.add(fmt::format("APPLICATION={}", options.application));
    }

    if (!options.language.empty()) {
        optionsArray.add(fmt::format("LANGUAGE={}", options.language));
    }

    _pimpl->gc = OGRGeocodeCreateSession(optionsArray.get());

    throw_on_invalid_handle();
}

Geocoder::~Geocoder()
{
    OGRGeocodeDestroySession(_pimpl->gc);
}

void Geocoder::allow_unsafe_ssl(bool enabled) noexcept
{
    _allowUnsafeSsl = enabled;
}

std::optional<Coordinate> Geocoder::geocode_single(const std::string& location, std::string_view countryCode)
{
    if (_allowUnsafeSsl) {
        CPLSetThreadLocalConfigOption("GDAL_HTTP_UNSAFESSL", "YES");
    }

    gdal::StringOptions optionsArray;
    if (!countryCode.empty()) {
        optionsArray.add(fmt::format("COUNTRYCODES={}", countryCode));
    }

    std::optional<Coordinate> result;

    auto layerHandle = OGRGeocode(_pimpl->gc, location.c_str(), nullptr, optionsArray.get());
    if (layerHandle) {
        ScopeGuard freeResult([layerHandle]() { OGRGeocodeFreeResult(layerHandle); });

        gdal::Layer layer(OGRLayer::FromHandle(layerHandle));
        for (auto& feature : layer) {
            if (feature.has_geometry()) {
                auto geom = feature.geometry();
                if (geom.type() == gdal::Geometry::Type::Point) {
                    return to_coordinate(geom.as<gdal::PointGeometry>().point());
                } else if (!result.has_value()) {
                    result = geom.centroid_coordinate();
                }
            }
        }
    }

    return result;
}

std::vector<Coordinate> Geocoder::geocode(const std::string& location, std::string_view countryCode)
{
    std::vector<Coordinate> result;

    gdal::StringOptions optionsArray;
    if (!countryCode.empty()) {
        optionsArray.add(fmt::format("COUNTRYCODES={}", countryCode));
    }

    
    auto layerHandle = OGRGeocode(_pimpl->gc, location.c_str(), nullptr, optionsArray.get());
    if (layerHandle) {
        ScopeGuard freeResult([layerHandle]() { OGRGeocodeFreeResult(layerHandle); });

        gdal::Layer layer(OGRLayer::FromHandle(layerHandle));
        for (auto& feature : layer) {
            if (feature.has_geometry()) {
                auto geom = feature.geometry();
                if (geom.type() == gdal::Geometry::Type::Point) {
                    result.push_back(to_coordinate(geom.as<gdal::PointGeometry>().point()));
                } else if (auto coord = geom.centroid_coordinate(); coord.has_value()) {
                    result.push_back(*coord);
                }
            }
        }
    }

    return result;
}

void Geocoder::throw_on_invalid_handle() const
{
    if (!_pimpl->gc) {
        throw RuntimeError("Failed to create geocoding session");
    }
}
}
