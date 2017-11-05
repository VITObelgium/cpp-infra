#pragma once

#include "infra/internal/gdalinternal.h"
#include "infra/point.h"

#include <algorithm>
#include <array>
#include <complex>
#include <fmt/format.h>
#include <gdal_priv.h>
#include <gsl/span>
//#include <optional>

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

void registerGdal();
void unregisterGdal();

enum class MapType
{
    Memory,
    ArcAscii,
    GeoTiff,
    Gif,
    Png,
    Unknown
};

enum class VectorType
{
    Csv,
    Tab,
    ShapeFile,
    Unknown
};

Point<double> convertPointProjected(int32_t sourceEpsg, int32_t destEpsg, Point<double> point);
Point<double> projectedToGeoGraphic(int32_t epsg, Point<double>);
std::string projectionToFriendlyName(const std::string& projection);
std::string projectionFromEpsg(int32_t epsg);
int32_t projectionToGeoEpsg(const std::string& projection);
int32_t projectionToEpsg(const std::string& projection);
std::vector<const char*> createOptionsArray(const std::vector<std::string>& driverOptions);

MapType guessMapTypeFromFileName(const std::string& filePath);
VectorType guessVectorTypeFromFileName(const std::string& filePath);

class DataSet
{
public:
    static DataSet create(const std::string& filePath);
    // if you know the type of the dataset, this will be faster as not all
    // drivers are queried
    static DataSet create(const std::string& filePath, VectorType type, const std::vector<std::string>& driverOptions = {});

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

    /*std::optional<double> noDataValue(int bandNr) const;
    void setNoDataValue(int bandNr, std::optional<double> value) const;*/

    void setColorTable(int bandNr, const GDALColorTable* ct);

    std::string projection() const;
    void setProjection(const std::string& proj);

    OGRLayer& getLayer(int index);

    GDALDataType getBandDataType(int index) const;

    template <typename T>
    void readRasterData(int band, int xOff, int yOff, int xSize, int ySize, T* pData, int bufXSize, int bufYSize) const
    {
        auto* bandPtr = _ptr->GetRasterBand(band);
        checkError(bandPtr->RasterIO(GF_Read, xOff, yOff, xSize, ySize, pData, bufXSize, bufYSize, type_resolve<T>::value, 0, 0),
            "Failed to read raster data");
    }

    template <typename T>
    void writeRasterData(int band, int xOff, int yOff, int xSize, int ySize, const T* pData, int bufXSize, int bufYSize) const
    {
        auto* bandPtr = _ptr->GetRasterBand(band);
        auto* dataPtr = const_cast<void*>(static_cast<const void*>(pData));
        checkError(bandPtr->RasterIO(GF_Write, xOff, yOff, xSize, ySize, dataPtr, bufXSize, bufYSize, type_resolve<T>::value, 0, 0),
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

        _ptr->AddBand(type_resolve<typename std::remove_cv<T>::type>::value, options.empty() ? nullptr : const_cast<char**>(options.data()));
    }

    template <typename T>
    void addBand(T* data, GDALColorInterp colorInterp)
    {
        addBand<T>(data);
        auto* band = _ptr->GetRasterBand(_ptr->GetRasterCount());
        band->SetColorInterpretation(colorInterp);
    }

    GDALDataset* get() const
    {
        return _ptr;
    }

private:
    explicit DataSet(const std::string& filename);

    GDALDataset* _ptr = nullptr;
};

class Driver
{
public:
    static Driver create(MapType);
    static Driver create(VectorType);
    static Driver create(const std::string& filename);

    template <typename T>
    DataSet createDataSet(uint32_t rows, uint32_t cols, uint32_t numBands, const std::string& filename)
    {
        return DataSet(checkPointer(_driver->Create(filename.c_str(), cols, rows, numBands, type_resolve<T>::value, nullptr), "Failed to create data set"));
    }

    template <typename T>
    DataSet createDataSetCopy(const DataSet& reference, const std::string& filename, const std::vector<std::string>& driverOptions = {})
    {
        auto options = createOptionsArray(driverOptions);
        return DataSet(checkPointer(_driver->CreateCopy(
                                        filename.c_str(),
                                        reference.get(),
                                        FALSE,
                                        options.size() == 1 ? nullptr : const_cast<char**>(options.data()),
                                        nullptr,
                                        nullptr),
            "Failed to create data set copy"));
    }

    MapType mapType() const;

private:
    explicit Driver(GDALDriver* driver)
    : _driver(driver)
    {
    }

    GDALDriver* _driver;
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
}
