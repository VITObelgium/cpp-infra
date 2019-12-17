#include "infra/geocoder.h"
#include "infra/conversion.h"
#include "infra/exception.h"
#include "infra/gdal.h"
#include "infra/gdalgeometry.h"

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
    std::vector<std::string> gcOptions;
    gcOptions.push_back(fmt::format("SERVICE={}", options.service));

    if (!options.cacheFile.empty()) {
        gcOptions.push_back(fmt::format("CACHE_FILE={}", options.cacheFile));
    }

    if (!options.readCache) {
        gcOptions.push_back("READ_CACHE=FALSE");
    }

    if (!options.writeCache) {
        gcOptions.push_back("WRITE_CACHE=FALSE");
    }

    if (!options.email.empty()) {
        gcOptions.push_back(fmt::format("EMAIL={}", options.email));
    }

    if (!options.key.empty()) {
        gcOptions.push_back(fmt::format("KEY={}", options.key));
    }

    if (!options.application.empty()) {
        gcOptions.push_back(fmt::format("APPLICATION={}", options.application));
    }

    if (!options.language.empty()) {
        gcOptions.push_back(fmt::format("LANGUAGE={}", options.language));
    }

    auto stringArray = gdal::create_string_array(gcOptions);
    _pimpl->gc       = OGRGeocodeCreateSession(const_cast<char**>(stringArray.data()));

    throw_on_invalid_handle();
}

Geocoder::~Geocoder()
{
    OGRGeocodeDestroySession(_pimpl->gc);
}

std::optional<Coordinate> Geocoder::geocode_single(const std::string& location, std::string_view countryCode)
{
    std::vector<std::string> gcOptions;
    if (!countryCode.empty()) {
        gcOptions.push_back(fmt::format("COUNTRYCODES={}", countryCode));
    }

    auto stringArray = gdal::create_string_array(gcOptions);
    auto layerHandle = OGRGeocode(_pimpl->gc, location.c_str(), nullptr, const_cast<char**>(stringArray.data()));
    if (layerHandle) {
        gdal::Layer layer(OGRLayer::FromHandle(layerHandle));
        if (layer.feature_count() > 0) {
            auto iter = begin(layer);
            if (iter->has_geometry()) {
                auto geom = iter->geometry();
                if (geom.type() == gdal::Geometry::Type::Point) {
                    return to_coordinate(geom.as<gdal::PointGeometry>().point());
                }
            }
        }

        OGRGeocodeFreeResult(layerHandle);
    }

    return {};
}

std::vector<Coordinate> Geocoder::geocode(const std::string& location, std::string_view countryCode)
{
    std::vector<Coordinate> result;

    std::vector<std::string> gcOptions;
    if (!countryCode.empty()) {
        gcOptions.push_back(fmt::format("COUNTRYCODES={}", countryCode));
    }

    auto stringArray = gdal::create_string_array(gcOptions);
    auto layerHandle = OGRGeocode(_pimpl->gc, location.c_str(), nullptr, const_cast<char**>(stringArray.data()));
    if (layerHandle) {
        gdal::Layer layer(OGRLayer::FromHandle(layerHandle));
        for (auto& feature : layer) {
            if (feature.has_geometry()) {
                auto geom = feature.geometry();
                if (geom.type() == gdal::Geometry::Type::Point) {
                    result.push_back(to_coordinate(geom.as<gdal::PointGeometry>().point()));
                }
            }
        }

        OGRGeocodeFreeResult(layerHandle);
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
