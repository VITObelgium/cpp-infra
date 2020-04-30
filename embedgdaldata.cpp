#include "embedgdaldata.h"
#include "gdal_data.h"
#include "infra/gdal.h"

#include <cpl_conv.h>

namespace inf::gdal {

using namespace std::string_view_literals;

struct MemoryFiles
{
    const MemoryFile gcs            = MemoryFile("/vsimem/data/gcs.csv", std::span<const uint8_t>(data::gcs));
    const MemoryFile pcs            = MemoryFile("/vsimem/data/pcs.csv", std::span<const uint8_t>(data::pcs));
    const MemoryFile gdalDatum      = MemoryFile("/vsimem/data/gdal_datum.csv", std::span<const uint8_t>(data::gdal_datum));
    const MemoryFile projopWparm    = MemoryFile("/vsimem/data/projop_wparm.csv", std::span<const uint8_t>(data::projop_wparm));
    const MemoryFile ellipsoid      = MemoryFile("/vsimem/data/ellipsoid.csv", std::span<const uint8_t>(data::ellipsoid));
    const MemoryFile coordinateAxis = MemoryFile("/vsimem/data/coordinate_axis.csv", std::span<const uint8_t>(data::coordinate_axis));
};

static std::unique_ptr<MemoryFiles> s_memFiles;

static const char* find_file(const char* /*className*/, const char* fileName)
{
    if (!s_memFiles) {
        return nullptr;
    }

    if ("gdal_datum.csv"sv == fileName || "datum.csv"sv == fileName) {
        return s_memFiles->gdalDatum.path().c_str();
    } else if ("gcs.csv"sv == fileName) {
        return s_memFiles->gcs.path().c_str();
    } else if ("pcs.csv"sv == fileName) {
        return s_memFiles->pcs.path().c_str();
    } else if ("projop_wparm.csv"sv == fileName) {
        return s_memFiles->projopWparm.path().c_str();
    } else if ("ellipsoid.csv"sv == fileName) {
        return s_memFiles->ellipsoid.path().c_str();
    } else if ("coordinate_axis.csv"sv == fileName) {
        return s_memFiles->coordinateAxis.path().c_str();
    }

    return nullptr;
}

void create_embedded_data()
{
    s_memFiles = std::make_unique<MemoryFiles>();
}

void destroy_embedded_data()
{
    s_memFiles.reset();
}

void register_embedded_data_file_finder()
{
    CPLPushFileFinder(find_file);
}

void unregister_embedded_data_file_finder()
{
    CPLPopFileFinder();
}
}
