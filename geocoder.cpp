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
    std::string proxy;
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
    _pimpl->proxy   = options.proxyServer;

    CPLStringList optionsArray;
    optionsArray.AddString(fmt::format("SERVICE={}", options.service).c_str());

    if (!options.cacheFile.empty()) {
        optionsArray.AddString(fmt::format("CACHE_FILE={}", options.cacheFile).c_str());
    }

    if (!options.readCache) {
        optionsArray.AddString("READ_CACHE=FALSE");
    }

    if (!options.writeCache) {
        optionsArray.AddString("WRITE_CACHE=FALSE");
    }

    if (!options.email.empty()) {
        optionsArray.AddString(fmt::format("EMAIL={}", options.email).c_str());
    }

    if (!options.key.empty()) {
        optionsArray.AddString(fmt::format("KEY={}", options.key).c_str());
    }

    if (!options.username.empty()) {
        optionsArray.AddString(fmt::format("USERNAME={}", options.key).c_str());
    }

    if (!options.application.empty()) {
        optionsArray.AddString(fmt::format("APPLICATION={}", options.application).c_str());
    }

    if (!options.language.empty()) {
        optionsArray.AddString(fmt::format("LANGUAGE={}", options.language).c_str());
    }

    _pimpl->gc = OGRGeocodeCreateSession(optionsArray.List());

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

    if (!_pimpl->proxy.empty()) {
        CPLSetThreadLocalConfigOption("GDAL_HTTP_PROXY", _pimpl->proxy.c_str());
    }

    CPLStringList optionsArray;
    if (!countryCode.empty()) {
        optionsArray.AddString(fmt::format("COUNTRYCODES={}", countryCode).c_str());
    }

    std::optional<Coordinate> result;

    auto layerHandle = OGRGeocode(_pimpl->gc, location.c_str(), nullptr, optionsArray.List());
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

    CPLStringList optionsArray;
    if (!countryCode.empty()) {
        optionsArray.AddString(fmt::format("COUNTRYCODES={}", countryCode).c_str());
    }

    auto layerHandle = OGRGeocode(_pimpl->gc, location.c_str(), nullptr, optionsArray.List());
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
