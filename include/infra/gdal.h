#pragma once

#include "infra/filesystem.h"
#include "infra/gdal-private.h"
#include "infra/gdalgeometry.h"
#include "infra/geometadata.h"
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

namespace inf::gdal {

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
    void transform_in_place(Point<double>& point) const;

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
Point<double> convert_point_projected(int32_t sourceEpsg, int32_t destEpsg, Point<double> point);
Point<double> projected_to_geographic(int32_t epsg, Point<double>);
std::string projection_to_friendly_name(const std::string& projection);
std::string projection_from_epsg(int32_t epsg);
int32_t projection_to_geo_epsg(const std::string& projection);
int32_t projection_to_epsg(const std::string& projection);
std::vector<const char*> create_options_array(const std::vector<std::string>& driverOptions);

RasterType guess_rastertype_from_filename(const fs::path& filePath);
VectorType guess_vectortype_from_filename(const fs::path& filePath);

void fill_geometadata_from_geo_transform(GeoMetadata& meta, const std::array<double, 6>& geoTrans);

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

    int32_t raster_count() const;

    int32_t x_size() const;
    int32_t y_size() const;
    std::array<double, 6> geotransform() const;
    void set_geotransform(const std::array<double, 6>& trans);

    std::optional<double> nodata_value(int bandNr) const;
    void set_nodata_value(int bandNr, std::optional<double> value) const;

    void set_colortable(int bandNr, const GDALColorTable* ct);

    std::string projection() const;
    void set_projection(const std::string& proj);

    void set_metadata(const std::string& name, const std::string& value, const std::string& domain = "");

    RasterBand rasterband(int index) const;

    GDALDataType band_datatype(int index) const;

    template <typename T>
    void read_rasterdata(int band, int xOff, int yOff, int xSize, int ySize, T* pData, int bufXSize, int bufYSize, int pixelSize = 0, int lineSize = 0) const
    {
        auto* bandPtr = _ptr->GetRasterBand(band);
        checkError(bandPtr->RasterIO(GF_Read, xOff, yOff, xSize, ySize, pData, bufXSize, bufYSize, TypeResolve<T>::value, pixelSize, lineSize),
            "Failed to read raster data");
    }

    template <typename T>
    void write_rasterdata(int band, int xOff, int yOff, int xSize, int ySize, const T* pData, int bufXSize, int bufYSize) const
    {
        auto* bandPtr = _ptr->GetRasterBand(band);
        auto* dataPtr = const_cast<void*>(static_cast<const void*>(pData));
        checkError(bandPtr->RasterIO(GF_Write, xOff, yOff, xSize, ySize, dataPtr, bufXSize, bufYSize, TypeResolve<T>::value, 0, 0),
            "Failed to write raster data");
    }

    void read_rasterdata(int band, int xOff, int yOff, int x_size, int y_size, const std::type_info& type, void* pData, int bufXSize, int bufYSize, int pixelSize = 0, int lineSize = 0) const;
    void write_rasterdata(int band, int xOff, int yOff, int x_size, int y_size, const std::type_info& type, const void* pData, int bufXSize, int bufYSize) const;

    void write_geometadata(const GeoMetadata& meta);
    GeoMetadata geometadata() const;

    void add_band(GDALDataType type, const void* data);

    template <typename T>
    void add_band(T* data)
    {
        add_band(TypeResolve<typename std::remove_cv<T>::type>::value, data);
    }

    template <typename T>
    void add_band(T* data, GDALColorInterp colorInterp)
    {
        add_band<T>(data);
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

    int32_t layer_count() const;

    std::string projection() const;
    void set_projection(const std::string& proj);

    void set_metadata(const std::string& name, const std::string& value, const std::string& domain = "");

    Layer layer(int index);
    Layer create_layer(const std::string& name, const std::vector<std::string>& driverOptions = {});
    Layer create_layer(const std::string& name, Geometry::Type type, const std::vector<std::string>& driverOptions = {});

    GDALDataset* get() const;
    VectorDriver driver();

private:
    GDALDataset* _ptr = nullptr;
};

class RasterDriver
{
public:
    static bool isSupported(RasterType);
    static RasterDriver create(RasterType);
    static RasterDriver create(const fs::path& filename);

    explicit RasterDriver(GDALDriver& driver);

    template <typename T>
    RasterDataSet create_dataset(int32_t rows, int32_t cols, int32_t numBands, const fs::path& filename)
    {
        return RasterDataSet(checkPointer(_driver.Create(filename.string().c_str(), cols, rows, numBands, TypeResolve<T>::value, nullptr), "Failed to create data set"));
    }

    RasterDataSet create_dataset(int32_t rows, int32_t cols, int32_t numBands, const fs::path& filename, const std::type_info& dataType);

    // Use for the memory driver, when there is no path
    template <typename T>
    RasterDataSet create_dataset(int32_t rows, int32_t cols, int32_t numBands)
    {
        return RasterDataSet(checkPointer(_driver.Create("", cols, rows, numBands, TypeResolve<T>::value, nullptr), "Failed to create data set"));
    }

    // Use for the memory driver, when there is no path
    RasterDataSet create_dataset(int32_t rows, int32_t cols, int32_t numBands, const std::type_info& dataType);

    RasterDataSet create_dataset_copy(const RasterDataSet& reference, const fs::path& filename, const std::vector<std::string>& driverOptions = {});

    RasterType type() const;

private:
    GDALDriver& _driver;
};

class VectorDriver
{
public:
    static bool isSupported(VectorType);
    static VectorDriver create(VectorType);
    static VectorDriver create(const fs::path& filename);

    explicit VectorDriver(GDALDriver& driver);

    // Use for the memory driver, when there is no path
    VectorDataSet create_dataset();
    VectorDataSet create_dataset(const fs::path& filename);
    VectorDataSet create_dataset_copy(const VectorDataSet& reference, const fs::path& filename, const std::vector<std::string>& driverOptions = {});

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

}
