#pragma once

#include "infra/coordinate.h"
#include "infra/filesystem.h"
#include "infra/gdal-private.h"
#include "infra/gdalgeometry.h"
#include "infra/gdalresample.h"
#include "infra/gdalspatialreference.h"
#include "infra/geometadata.h"
#include "infra/point.h"
#include "infra/span.h"

#include <algorithm>
#include <array>
#include <complex>
#include <fmt/format.h>
#include <gdal_priv.h>
#include <ogr_feature.h>

#include <chrono>
#include <optional>
#include <unordered_map>
#include <variant>

namespace inf::gdal {

using namespace std::string_literals;

struct RegistrationConfig
{
    bool setLogHandler = true;

    // In case you need coordinate transformations, pass the path to the proj.db file (gdal > 3.0)
    fs::path projdbPath;
};

// RAII wrapper for gdal registration (only instantiate this once in your application)
class Registration
{
public:
    Registration();
    Registration(RegistrationConfig cfg);
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
void register_gdal();
void register_gdal(RegistrationConfig cfg);
void unregister_gdal();

// Call this on each thread that requires access to the proj.4 data
// Not needed on the thread that did the gdal registration
void register_embedded_data();
// In case you need coordinate transformations, pass the path to the proj.db file (gdal > 3.0)
void register_embedded_data(const fs::path& p);
void unregister_embedded_data();

std::string get_memory_file_buffer(const fs::path& p, bool remove);

class RasterDriver;
class VectorDriver;

enum class FileType
{
    Raster,
    Vector,
    Unknown,
};

enum class RasterType
{
    Memory,
    ArcAscii,
    GeoTiff,
    Gif,
    Png,
    PcRaster,
    Netcdf,
    TileDB,
    MBTiles,
    GeoPackage,
    Grib,
    Postgis,
    Vrt,
    Unknown,
};

enum class VectorType
{
    Memory,
    Csv,
    Tab,
    ShapeFile,
    Xlsx,
    GeoJson,
    GeoPackage,
    PostgreSQL,
    WFS,
    Vrt,
    Unknown,
};

RasterType guess_rastertype_from_filename(const fs::path& filePath);
VectorType guess_vectortype_from_filename(const fs::path& filePath);

void fill_geometadata_from_geo_transform(GeoMetadata& meta, const std::array<double, 6>& geoTrans);

class RasterBand
{
public:
    RasterBand(GDALRasterBand* ptr);

    GDALRasterBand* get();
    const GDALRasterBand* get() const;

    const std::type_info& datatype() const;
    inf::Size block_size();
    int32_t overview_count();
    RasterBand overview_dataset(int index);

    int32_t x_size();
    int32_t y_size();

private:
    GDALRasterBand* _band;
};

enum class OpenMode
{
    ReadOnly,
    ReadWrite,
};

struct RasterStats
{
    double min    = std::numeric_limits<double>::quiet_NaN();
    double max    = std::numeric_limits<double>::quiet_NaN();
    double mean   = std::numeric_limits<double>::quiet_NaN();
    double stddev = std::numeric_limits<double>::quiet_NaN();
};

class RasterDataSet
{
public:
    [[deprecated("use open")]] static RasterDataSet create(const fs::path& filePath, const std::vector<std::string>& driverOpts = {});
    [[deprecated("use open")]] static RasterDataSet create(const fs::path& filePath, RasterType type, const std::vector<std::string>& driverOpts = {});

    static RasterDataSet open(const fs::path& filePath, const std::vector<std::string>& driverOpts = {});
    static RasterDataSet open(const fs::path& filePath, RasterType type, const std::vector<std::string>& driverOpts = {});

    static RasterDataSet open_for_writing(const fs::path& filePath, const std::vector<std::string>& driverOpts = {});
    static RasterDataSet open_for_writing(const fs::path& filePath, RasterType type, const std::vector<std::string>& driverOpts = {});

    RasterDataSet() = default;
    explicit RasterDataSet(GDALDatasetH ptr) noexcept;
    explicit RasterDataSet(GDALDataset* ptr) noexcept;

    RasterDataSet(RasterDataSet&&) noexcept;
    ~RasterDataSet() noexcept;

    RasterDataSet& operator=(RasterDataSet&&) noexcept;

    bool is_valid() const noexcept;
    int32_t raster_count() const;

    int32_t x_size() const;
    int32_t y_size() const;
    bool has_valid_geotransform() const;
    std::array<double, 6> geotransform() const;
    void set_geotransform(const std::array<double, 6>& trans);

    std::optional<double> nodata_value(int bandNr) const;
    void set_nodata_value(int bandNr, std::optional<double> value) const;

    void set_colortable(int bandNr, const GDALColorTable* ct);

    std::string projection() const;
    void set_projection(const std::string& proj);

    std::string metadata_item(const std::string& name, const std::string& domain = "");
    std::string band_metadata_item(int bandNr, const std::string& name, const std::string& domain = "");
    std::unordered_map<std::string, std::string> metadata(const std::string& domain = "");
    void set_metadata(const std::string& name, const std::string& value, const std::string& domain = "");
    void set_band_metadata(int bandNr, const std::string& name, const std::string& value, const std::string& domain = "");
    void set_band_description(int bandNr, const std::string& description);
    std::vector<std::string> metadata_domains() const noexcept;

    RasterType type() const;

    const std::type_info& band_datatype(int index) const;
    RasterBand rasterband(int index) const;

    template <typename T>
    void read_rasterdata(int band, int xOff, int yOff, int xSize, int ySize, T* pData, int bufXSize, int bufYSize, int pixelSize = 0, int lineSize = 0) const
    {
        auto* bandPtr = _ptr->GetRasterBand(band);
        check_error(bandPtr->RasterIO(GF_Read, xOff, yOff, xSize, ySize, pData, bufXSize, bufYSize, TypeResolve<T>::value, pixelSize, lineSize),
                    "Failed to read raster data");
    }

    /*
     * Convenience method that returns the full raster band as a vector
     */
    template <typename T>
    std::vector<T> read_rasterdata(int band) const
    {
        const auto xSize = x_size();
        const auto ySize = y_size();

        std::vector<T> result(xSize * ySize);
        read_rasterdata<T>(band, 0, 0, xSize, ySize, result.data(), xSize, ySize);
        return result;
    }

    template <typename T>
    void write_rasterdata(int band, int xOff, int yOff, int xSize, int ySize, const T* pData, int bufXSize, int bufYSize) const
    {
        auto* bandPtr = check_pointer(_ptr->GetRasterBand(band), "Failed to get raster band for writing");
        auto* dataPtr = const_cast<void*>(static_cast<const void*>(pData));
        check_error(bandPtr->RasterIO(GF_Write, xOff, yOff, xSize, ySize, dataPtr, bufXSize, bufYSize, TypeResolve<T>::value, 0, 0),
                    "Failed to write raster data");
    }

    void read_rasterdata(int band, int xOff, int yOff, int x_size, int y_size, const std::type_info& type, void* pData, int bufXSize, int bufYSize, int pixelSize = 0, int lineSize = 0) const;
    void write_rasterdata(int band, int xOff, int yOff, int x_size, int y_size, const std::type_info& type, const void* pData, int bufXSize, int bufYSize) const;

    void write_geometadata(const GeoMetadata& meta);
    GeoMetadata geometadata() const;
    GeoMetadata geometadata(int bandNr) const;

    void flush_cache();

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

    RasterStats statistics(int bandNr, bool allowApproximation, bool force);
    void build_overviews(ResampleAlgorithm resample, std::span<const int32_t> levels);

    GDALDataset* get() const;
    RasterDriver driver();

private:
    RasterDriver driver() const;

    GDALDataset* _ptr = nullptr;
};

class VectorDataSet
{
public:
    [[deprecated("use open")]] static VectorDataSet create(const fs::path& filePath, const std::vector<std::string>& driverOptions = {});
    [[deprecated("use open")]] static VectorDataSet create(const fs::path& filePath, VectorType type, const std::vector<std::string>& driverOptions = {});

    static VectorDataSet open(const fs::path& filename, const std::vector<std::string>& driverOptions = {});
    static VectorDataSet open(const fs::path& filename, VectorType type, const std::vector<std::string>& driverOptions = {});

    static VectorDataSet open_for_writing(const fs::path& filename, const std::vector<std::string>& driverOptions = {});
    static VectorDataSet open_for_writing(const fs::path& filename, VectorType type, const std::vector<std::string>& driverOptions = {});

    VectorDataSet() = default;
    explicit VectorDataSet(GDALDataset* ptr) noexcept;

    VectorDataSet(VectorDataSet&&) noexcept;
    ~VectorDataSet() noexcept;

    VectorDataSet& operator=(VectorDataSet&&);

    bool is_valid() const;
    int32_t layer_count() const;

    std::string projection() const;
    void set_projection(const std::string& proj);

    void set_metadata(const std::string& name, const std::string& value, const std::string& domain = "");

    Layer layer(int index);
    Layer layer(const std::string& name);

    Layer create_layer(const std::string& name, const std::vector<std::string>& driverOptions = {});
    Layer create_layer(const std::string& name, Geometry::Type type, const std::vector<std::string>& driverOptions = {});
    Layer copy_layer(Layer srcLayer, const char* newLayerName, const std::vector<std::string>& driverOptions = {});

    /*! Make sure to keep the spatial reference object alive while working with the dataset! */
    Layer create_layer(const std::string& name, SpatialReference& spatialRef, Geometry::Type type, const std::vector<std::string>& driverOptions = {});

    void start_transaction();
    void commit_transaction();
    void rollback_transaction();

    GDALDataset* get() const;
    VectorDriver driver();

private:
    GDALDataset* _ptr = nullptr;
};

class DatasetTransaction
{
public:
    DatasetTransaction(VectorDataSet& ds) noexcept
    : _ds(ds)
    {
        _ds.start_transaction();
    }

    ~DatasetTransaction()
    {
        if (_rollbackNeeded) {
            _ds.rollback_transaction();
        }
    }

    void commit()
    {
        _ds.commit_transaction();
        _rollbackNeeded = false;
    }

private:
    VectorDataSet& _ds;
    bool _rollbackNeeded = true;
};

class RasterDriver
{
public:
    static bool is_supported(RasterType);
    static RasterDriver create(RasterType);
    static RasterDriver create(const fs::path& filename);

    explicit RasterDriver(GDALDriver& driver);

    template <typename T>
    RasterDataSet create_dataset(int32_t rows, int32_t cols, int32_t numBands, const fs::path& filename, std::span<const std::string> driverOptions = {})
    {
        auto options = create_string_list(driverOptions);
        return RasterDataSet(check_pointer(_driver.Create(filename.string().c_str(), cols, rows, numBands, TypeResolve<T>::value, options.List()), "Failed to create data set"));
    }

    RasterDataSet create_dataset(int32_t rows, int32_t cols, int32_t numBands, const fs::path& filename, const std::type_info& dataType);

    // Use for the memory driver, when there is no path
    template <typename T>
    RasterDataSet create_dataset(int32_t rows, int32_t cols, int32_t numBands, std::span<const std::string> driverOptions = {})
    {
        auto options = create_string_list(driverOptions);
        return RasterDataSet(check_pointer(_driver.Create("", cols, rows, numBands, TypeResolve<T>::value, options.List()), "Failed to create data set"));
    }

    // Use for the memory driver, when there is no path
    RasterDataSet create_dataset(int32_t rows, int32_t cols, int32_t numBands, const std::type_info& dataType);

    RasterDataSet create_dataset_copy(const RasterDataSet& reference, const fs::path& filename, std::span<const std::string> driverOptions = {});

    RasterType type() const;

private:
    GDALDriver& _driver;
};

class VectorDriver
{
public:
    static bool is_supported(VectorType);
    static VectorDriver create(VectorType);
    static VectorDriver create(const fs::path& filename);

    explicit VectorDriver(GDALDriver& driver);

    // Use for the memory driver, when there is no path
    VectorDataSet create_dataset();
    VectorDataSet create_dataset(const fs::path& filename, const std::vector<std::string>& creationOptions = {});
    void delete_dataset(const fs::path& filename);

    VectorType type() const;

private:
    GDALDriver& _driver;
};

FileType detect_file_type(const fs::path& path);

class MemoryFile
{
public:
    MemoryFile(const char* path, std::span<const uint8_t> dataBuffer);
    MemoryFile(const char* path, std::string_view dataBuffer);

    MemoryFile(std::string path, std::span<const uint8_t> dataBuffer);
    MemoryFile(std::string path, std::string_view dataBuffer);

    MemoryFile(const fs::path& path, std::span<const uint8_t> dataBuffer);
    MemoryFile(const fs::path& path, std::string_view dataBuffer);

    MemoryFile(MemoryFile&&) noexcept;
    MemoryFile(const MemoryFile&) = delete;

    ~MemoryFile() noexcept;

    MemoryFile& operator=(MemoryFile&&) noexcept;
    MemoryFile& operator=(const MemoryFile&) = delete;

    const std::string& path() const;
    std::span<uint8_t> data();

private:
    void close() noexcept;

    std::string _path;
    VSILFILE* _ptr;
};

enum class MemoryReadMode
{
    LeaveIntact,
    StealContents,
};

std::string read_memory_file_as_text(const fs::path& path, MemoryReadMode mode = MemoryReadMode::LeaveIntact);

}
