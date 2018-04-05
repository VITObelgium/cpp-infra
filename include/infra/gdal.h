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
class RasterDriver;
class VectorDriver;

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

    OGRCoordinateTransformation* get();

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

RasterType guessRasterTypeFromFileName(const fs::path& filePath);
VectorType guessVectorTypeFromFileName(const fs::path& filePath);

class RasterBand
{
public:
    RasterBand(GDALRasterBand* ptr);

    GDALRasterBand* get();
    const GDALRasterBand* get() const;

private:
    GDALRasterBand* _band;
};

class RasterDataSet
{
public:
    static RasterDataSet create(const fs::path& filePath, const std::vector<std::string>& driverOpts = {});
    static RasterDataSet create(const fs::path& filePath, RasterType type, const std::vector<std::string>& driverOpts = {});

    RasterDataSet() = default;
    explicit RasterDataSet(GDALDataset* ptr) noexcept;

    RasterDataSet(RasterDataSet&&);
    ~RasterDataSet() noexcept;

    RasterDataSet& operator=(RasterDataSet&&);

    int32_t rasterCount() const;

    int32_t xSize() const;
    int32_t ySize() const;
    std::array<double, 6> geoTransform() const;
    void setGeoTransform(const std::array<double, 6>& trans);

    std::optional<double> noDataValue(int bandNr) const;
    void setNoDataValue(int bandNr, std::optional<double> value) const;

    void setColorTable(int bandNr, const GDALColorTable* ct);

    std::string projection() const;
    void setProjection(const std::string& proj);

    void setMetadata(const std::string& name, const std::string& value, const std::string& domain = "");

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

    void readRasterData(int band, int xOff, int yOff, int xSize, int ySize, const std::type_info& type, void* pData, int bufXSize, int bufYSize, int pixelSize = 0, int lineSize = 0) const;
    void writeRasterData(int band, int xOff, int yOff, int xSize, int ySize, const std::type_info& type, const void* pData, int bufXSize, int bufYSize) const;

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
    RasterDriver driver();

private:
    GDALDataset* _ptr = nullptr;
};

class VectorDataSet
{
public:
    static VectorDataSet create(const fs::path& filePath, const std::vector<std::string>& driverOptions = {});
    static VectorDataSet create(const fs::path& filePath, VectorType type, const std::vector<std::string>& driverOptions = {});

    VectorDataSet() = default;
    explicit VectorDataSet(GDALDataset* ptr) noexcept;

    VectorDataSet(VectorDataSet&&);
    ~VectorDataSet() noexcept;

    VectorDataSet& operator=(VectorDataSet&&);

    int32_t layerCount() const;

    std::string projection() const;
    void setProjection(const std::string& proj);

    void setMetadata(const std::string& name, const std::string& value, const std::string& domain = "");

    Layer getLayer(int index);
    Layer createLayer(const std::string& name, const std::vector<std::string>& driverOptions = {});
    Layer createLayer(const std::string& name, Geometry::Type type, const std::vector<std::string>& driverOptions = {});

    GDALDataset* get() const;
    VectorDriver driver();

private:
    GDALDataset* _ptr = nullptr;
};

class RasterDriver
{
public:
    static RasterDriver create(RasterType);
    static RasterDriver create(const fs::path& filename);

    explicit RasterDriver(GDALDriver& driver);

    template <typename T>
    RasterDataSet createDataSet(int32_t rows, int32_t cols, int32_t numBands, const fs::path& filename)
    {
        return RasterDataSet(checkPointer(_driver.Create(filename.string().c_str(), cols, rows, numBands, TypeResolve<T>::value, nullptr), "Failed to create data set"));
    }

    // Use for the memory driver, when there is no path
    template <typename T>
    RasterDataSet createDataSet(int32_t rows, int32_t cols, int32_t numBands)
    {
        return RasterDataSet(checkPointer(_driver.Create("", cols, rows, numBands, TypeResolve<T>::value, nullptr), "Failed to create data set"));
    }

    template <typename T>
    RasterDataSet createDataSetCopy(const RasterDataSet& reference, const fs::path& filename, const std::vector<std::string>& driverOptions = {})
    {
        auto options = createOptionsArray(driverOptions);
        return RasterDataSet(checkPointer(_driver.CreateCopy(
                                              filename.string().c_str(),
                                              reference.get(),
                                              FALSE,
                                              options.size() == 1 ? nullptr : const_cast<char**>(options.data()),
                                              nullptr,
                                              nullptr),
            "Failed to create data set copy"));
    }

    RasterType type() const;

private:
    GDALDriver& _driver;
};

class VectorDriver
{
public:
    static VectorDriver create(VectorType);
    static VectorDriver create(const fs::path& filename);

    explicit VectorDriver(GDALDriver& driver);

    // Use for the memory driver, when there is no path
    VectorDataSet createDataSet();
    VectorDataSet createDataSet(const fs::path& filename);
    VectorDataSet createDataSetCopy(const VectorDataSet& reference, const fs::path& filename, const std::vector<std::string>& driverOptions = {});

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

infra::GeoMetadata readMetadataFromDataset(const gdal::RasterDataSet& dataSet);
}
