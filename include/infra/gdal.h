#pragma once

#include "infra/filesystem.h"
#include "infra/gdalgeometry.h"
#include "infra/geometadata.h"
#include "infra/internal/gdalinternal.h"
#include "infra/point.h"

#include <algorithm>
#include <array>
#include <complex>
#include <fmt/format.h>
#include <gdal_priv.h>
#include <gsl/span>
#include <ogr_feature.h>
#include <ogr_spatialref.h>

#include <chrono>
#include <optional>
#include <variant>

namespace infra::gdal {

using namespace std::string_literals;

// RAII wrapper for gdal registration
class Registration
{
public:
    Registration();
    ~Registration();
};

// Has to be instantiated on each thread that requires access to the proj.4 data
class EmbeddedDataRegistration
{
public:
    EmbeddedDataRegistration();
    ~EmbeddedDataRegistration();
};

// Free function versions of the registration handling
// Call this ones in each application that wishes to use gdal
void registerGdal();
void unregisterGdal();

// Call this on each thread that requires access to the proj.4 data
// Not needed on the thread that did the gdal registration
void registerEmbeddedData();
void unregisterEmbeddedData();

class Layer;
class Driver;

enum class RasterType
{
    Memory,
    ArcAscii,
    GeoTiff,
    Gif,
    Png,
    PcRaster,
    Unknown
};

enum class VectorType
{
    Memory,
    Csv,
    Tab,
    ShapeFile,
    Xlsx,
    Unknown
};

class CoordinateTransformer
{
public:
    CoordinateTransformer(int32_t sourceEpsg, int32_t destEpsg);

    Point<double> transform(const Point<double>& point) const;
    void transformInPlace(Point<double>& point) const;

private:
    OGRSpatialReference _sourceSRS;
    OGRSpatialReference _targetSRS;
    std::unique_ptr<OGRCoordinateTransformation> _transformer;
};

/* convenience function to convert a single point (internally creates a CoordinateTransformer)
 * Don't use this function for converting a lot of points as there is a significant overhead
 * in creating a CoordinateTransformer instance for every point
 */
Point<double> convertPointProjected(int32_t sourceEpsg, int32_t destEpsg, Point<double> point);
Point<double> projectedToGeoGraphic(int32_t epsg, Point<double>);
std::string projectionToFriendlyName(const std::string& projection);
std::string projectionFromEpsg(int32_t epsg);
int32_t projectionToGeoEpsg(const std::string& projection);
int32_t projectionToEpsg(const std::string& projection);
std::vector<const char*> createOptionsArray(const std::vector<std::string>& driverOptions);

RasterType guessRasterTypeFromFileName(const std::string& filePath);
VectorType guessVectorTypeFromFileName(const std::string& filePath);

class RasterBand
{
public:
    RasterBand(GDALRasterBand* ptr);

    GDALRasterBand* get();
    const GDALRasterBand* get() const;

private:
    GDALRasterBand* _band;
};

class DataSet
{
public:
    // This can only be used for raster types
    static DataSet createRaster(const std::string& filePath, const std::vector<std::string>& driverOpts = {});
    static DataSet createRaster(const std::string& filePath, RasterType type, const std::vector<std::string>& driverOpts = {});
    // if you know the type of the dataset, this will be faster as not all drivers are queried
    static DataSet openVector(const std::string& filePath, const std::vector<std::string>& driverOptions = {});
    static DataSet openVector(const std::string& filePath, VectorType type, const std::vector<std::string>& driverOptions = {});

    DataSet() = default;
    explicit DataSet(GDALDataset* ptr) noexcept;

    DataSet(DataSet&&);
    ~DataSet() noexcept;

    DataSet& operator=(DataSet&&);

    int32_t rasterCount() const;
    int32_t layerCount() const;

    int32_t rasterXSize() const;
    int32_t rasterYSize() const;
    std::array<double, 6> geoTransform() const;
    void setGeoTransform(const std::array<double, 6>& trans);

    std::optional<double> noDataValue(int bandNr) const;
    void setNoDataValue(int bandNr, std::optional<double> value) const;

    void setColorTable(int bandNr, const GDALColorTable* ct);

    std::string projection() const;
    void setProjection(const std::string& proj);

    void setMetadata(const std::string& name, const std::string& value, const std::string& domain = "");

    Layer getLayer(int index);
    Layer createLayer(const std::string& name, const std::vector<std::string>& driverOptions = {});
    Layer createLayer(const std::string& name, Geometry::Type type, const std::vector<std::string>& driverOptions = {});

    RasterBand rasterBand(int index) const;

    GDALDataType getBandDataType(int index) const;

    template <typename T>
    void readRasterData(int band, int xOff, int yOff, int xSize, int ySize, T* pData, int bufXSize, int bufYSize, int pixelSize = 0, int lineSize = 0) const
    {
        auto* bandPtr = _ptr->GetRasterBand(band);
        checkError(bandPtr->RasterIO(GF_Read, xOff, yOff, xSize, ySize, pData, bufXSize, bufYSize, TypeResolve<T>::value, pixelSize, lineSize),
            "Failed to read raster data");
    }

    template <typename T>
    void writeRasterData(int band, int xOff, int yOff, int xSize, int ySize, const T* pData, int bufXSize, int bufYSize) const
    {
        auto* bandPtr = _ptr->GetRasterBand(band);
        auto* dataPtr = const_cast<void*>(static_cast<const void*>(pData));
        checkError(bandPtr->RasterIO(GF_Write, xOff, yOff, xSize, ySize, dataPtr, bufXSize, bufYSize, TypeResolve<T>::value, 0, 0),
            "Failed to write raster data");
    }

    template <typename T>
    void addBand(T* data)
    {
        // convert the data pointer to a string
        std::array<char, 32> buf;
        auto writtenCharacters = CPLPrintPointer(buf.data(), const_cast<void*>(reinterpret_cast<const void*>(data)), static_cast<int>(buf.size()));
        buf[writtenCharacters] = 0;

        auto pointerString = fmt::format("DATAPOINTER={}", buf.data());
        std::array<const char*, 2> options{{pointerString.c_str(), nullptr}};

        _ptr->AddBand(TypeResolve<typename std::remove_cv<T>::type>::value, options.empty() ? nullptr : const_cast<char**>(options.data()));
    }

    template <typename T>
    void addBand(T* data, GDALColorInterp colorInterp)
    {
        addBand<T>(data);
        auto* band = _ptr->GetRasterBand(_ptr->GetRasterCount());
        band->SetColorInterpretation(colorInterp);
    }

    GDALDataset* get() const;
    Driver driver();

private:
    static GDALDataset* create(const std::string& filename,
        unsigned int openFlags,
        const char* const* drivers,
        const std::vector<std::string>& driverOpts);
    explicit DataSet(const std::string& filename);

    GDALDataset* _ptr = nullptr;
};

class Driver
{
public:
    static Driver create(RasterType);
    static Driver create(VectorType);
    static Driver create(const std::string& filename);

    explicit Driver(GDALDriver& driver);

    template <typename T>
    DataSet createDataSet(uint32_t rows, uint32_t cols, uint32_t numBands, const std::string& filename)
    {
        return DataSet(checkPointer(_driver.Create(filename.c_str(), cols, rows, numBands, TypeResolve<T>::value, nullptr), "Failed to create data set"));
    }

    template <typename T>
    DataSet createDataSetCopy(const DataSet& reference, const std::string& filename, const std::vector<std::string>& driverOptions = {})
    {
        auto options = createOptionsArray(driverOptions);
        return DataSet(checkPointer(_driver.CreateCopy(
                                        filename.c_str(),
                                        reference.get(),
                                        FALSE,
                                        options.size() == 1 ? nullptr : const_cast<char**>(options.data()),
                                        nullptr,
                                        nullptr),
            "Failed to create data set copy"));
    }

    RasterType rasterType() const;
    VectorType vectorType() const;

private:
    GDALDriver& _driver;
};

class VectorDriver
{
public:
    static VectorDriver create(VectorType);
    static VectorDriver create(const fs::path& filename);
    explicit VectorDriver(GDALDriver& driver);

    DataSet createDataSet(const fs::path& filename);
    DataSet createDataSetCopy(const DataSet& reference, const fs::path& filename, const std::vector<std::string>& driverOptions = {});

    VectorType type() const;

private:
    GDALDriver& _driver;
};

class MemoryFile
{
public:
    MemoryFile(std::string path, gsl::span<const uint8_t> dataBuffer);
    ~MemoryFile();

    const std::string& path() const;

private:
    const std::string _path;
    VSILFILE* _ptr;
};

infra::GeoMetadata readMetadataFromDataset(const gdal::DataSet& dataSet);
}
