#include "embedgdaldata.h"
#include "gdal_data.h"
#include "infra/gdal.h"
#include "infra/log.h"

#include <cpl_conv.h>

namespace infra::gdal {

using namespace std::string_literals;

struct MemoryFiles
{
    const MemoryFile gcs            = MemoryFile("/vsimem/data/gcs.csv", gsl::span<const uint8_t>(data::gcs));
    const MemoryFile pcs            = MemoryFile("/vsimem/data/pcs.csv", gsl::span<const uint8_t>(data::pcs));
    const MemoryFile gdalDatum      = MemoryFile("/vsimem/data/gdal_datum.csv", gsl::span<const uint8_t>(data::gdal_datum));
    const MemoryFile ellipsoid      = MemoryFile("/vsimem/data/ellipsoid.csv", gsl::span<const uint8_t>(data::ellipsoid));
    const MemoryFile coordinateAxis = MemoryFile("/vsimem/data/coordinate_axis.csv", gsl::span<const uint8_t>(data::coordinate_axis));
};

static std::unique_ptr<MemoryFiles> s_memFiles;

static const char* findFile(const char* className, const char* fileName)
{
    if (!s_memFiles) {
        return nullptr;
    }

    Log::debug("{} : {}", className, fileName);

    //if ("epsdata::csv"s == className) {
    if ("gdal_datum.csv"s == fileName || "datum.csv"s == fileName) {
        return s_memFiles->gdalDatum.path().c_str();
    } else if ("gcs.csv"s == fileName) {
        return s_memFiles->gcs.path().c_str();
    } else if ("pcs.csv"s == fileName) {
        return s_memFiles->pcs.path().c_str();
    } else if ("ellipsoid.csv"s == fileName) {
        return s_memFiles->ellipsoid.path().c_str();
    } else if ("coordinate_axis.csv"s == fileName) {
        return s_memFiles->coordinateAxis.path().c_str();
    }
    //}

    return nullptr;
}

void createEmbeddedData()
{
    s_memFiles = std::make_unique<MemoryFiles>();
    CPLPushFileFinder(findFile);
}

void destroyEmbeddedData()
{
    s_memFiles.reset();
}
}