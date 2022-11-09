#pragma once

#include "infra/coordinate.h"
#include "infra/filesystem.h"

#include <memory>
#include <optional>
#include <string>

namespace inf {

class Geocoder
{
public:
    static std::string Osm;
    static std::string MapQuest;
    static std::string GeoNames;
    static std::string Bing;

    struct Options
    {
        fs::path cacheFile;
        bool readCache      = true;
        bool writeCache     = true;
        bool allowUnsafeSsl = false;
        std::string proxyServer;
        std::string service = Osm;
        std::string email;       // optional, but recommended
        std::string key;         // api key, needed by Bing
        std::string username;    // username, needed by GeoNames
        std::string application; // user-agent
        std::string language;    // prefered language of the resutl (when reverse geocoding)
    };

    Geocoder();
    Geocoder(const Options& options);
    ~Geocoder();

    void allow_unsafe_ssl(bool enabled = true) noexcept;

    std::optional<Coordinate> geocode_single(const std::string& location, std::string_view countryCode = {});
    std::vector<Coordinate> geocode(const std::string& location, std::string_view countryCode = {});

private:
    void throw_on_invalid_handle() const;

    struct Pimpl;
    bool _allowUnsafeSsl = false;
    std::unique_ptr<Pimpl> _pimpl;
};

}
