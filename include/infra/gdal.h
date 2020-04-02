#pragma once

#include "infra/coordinate.h"
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
#include <unordered_map>
#include <variant>

namespace inf::gdal {

using namespace std::string_literals;

// RAII wrapper for gdal registration
class Registration
{
public:
    Registration();
    // In case you need coordinate transformations, pass the path to the proj.db file
    Registration(const fs::path& p);
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
// In case you need coordinate transformations, pass the path to the proj.db file
void register_gdal(const fs::path& p);
void unregister_gdal();

// Call this on each thread that requires access to the proj.4 data
// Not needed on the thread that did the gdal registration
void register_embedded_data();
void register_embedded_data(const fs::path& p);
void unregister_embedded_data();

std::string get_memory_file_buffer(const fs::path& p, bool remove);

class Layer;
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
    Unknown,
};

/*! Wrapper around the OGRSpatialReference class
 * Needed as the OGRSpatialReference destructor is deprecated
 * so it is very easy to misuse the OGRSpatialReference API
 */
class SpatialReference
{
public:
    SpatialReference();
    SpatialReference(int32_t epsg);
    SpatialReference(const char* wkt);
    SpatialReference(const std::string& wkt);
    SpatialReference(OGRSpatialReference* instance);
    ~SpatialReference() noexcept;

    SpatialReference(SpatialReference&&);

    SpatialReference clone() const;
    SpatialReference clone_geo_cs() const;

    void import_from_epsg(int32_t epsg);
    void import_from_wkt(const char* wkt);

    std::string export_to_wkt() const;
    std::string export_to_pretty_wkt() const;
    std::string export_to_pretty_wkt_simplified() const;

    std::optional<int32_t> epsg_cs() const noexcept;
    std::optional<int32_t> epsg_geog_cs() const noexcept;
    std::string_view authority_code(const char* key) const noexcept;

    void set_proj_cs(const char* projCs);
    void set_well_known_geog_cs(const char* geogCs);
    void set_utm(int zone, bool north = true);

    bool is_geographic() const;
    bool is_projected() const;

    OGRSpatialReference* get() noexcept;
    const OGRSpatialReference* get() const noexcept;

private:
    OGRSpatialReference* _srs = nullptr;
};

class CoordinateTransformer
{
public:
    CoordinateTransformer(SpatialReference source, SpatialReference dest);
    CoordinateTransformer(int32_t sourceEpsg, int32_t destEpsg);

    Point<double> transform(const Point<double>& point) const;
    void transform_in_place(Point<double>& point) const;

    Coordinate transform(const Coordinate& coord) const;
    void transform_in_place(Coordinate& coord) const;

    std::string source_projection() const;
    std::string target_projection() const;

    OGRCoordinateTransformation* get();

private:
    SpatialReference _sourceSRS;
    SpatialReference _targetSRS;
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
std::optional<int32_t> projection_to_geo_epsg(const std::string& projection) noexcept;
std::optional<int32_t> projection_to_epsg(const std::string& projection) noexcept;
CPLStringList create_string_list(gsl::span<const std::string> driverOptions);

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

enum class OpenMode
{
    ReadOnly,
    ReadWrite,
};

class RasterDataSet
{
public:
    static RasterDataSet create(const fs::path& filePath, const std::vector<std::string>& driverOpts = {});
    static RasterDataSet create(const fs::path& filePath, RasterType type, const std::vector<std::string>& driverOpts = {});

    static RasterDataSet open_for_writing(const fs::path& filePath, const std::vector<std::string>& driverOpts = {});
    static RasterDataSet open_for_writing(const fs::path& filePath, RasterType type, const std::vector<std::string>& driverOpts = {});

    RasterDataSet() = default;
    explicit RasterDataSet(GDALDataset* ptr) noexcept;

    RasterDataSet(RasterDataSet&&);
    ~RasterDataSet() noexcept;

    RasterDataSet& operator=(RasterDataSet&&);

    bool is_valid() const;
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
    std::vector<std::string> metadata_domains() const noexcept;

    RasterBand rasterband(int index) const;

    RasterType type() const;

    const std::type_info& band_datatype(int index) const;

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

    GDALDataset* get() const;
    RasterDriver driver();

private:
    RasterDriver driver() const;

    GDALDataset* _ptr = nullptr;
};

class VectorDataSet
{
public:
    [[deprecated("use open")]]
    static VectorDataSet create(const fs::path& filePath, const std::vector<std::string>& driverOptions = {});
    [[deprecated("use open")]]
    static VectorDataSet create(const fs::path& filePath, VectorType type, const std::vector<std::string>& driverOptions = {});

    static VectorDataSet open(const fs::path& filename, OpenMode mode = OpenMode::ReadOnly, const std::vector<std::string>& driverOptions = {});
    static VectorDataSet open(const fs::path& filename, VectorType type, OpenMode mode = OpenMode::ReadOnly, const std::vector<std::string>& driverOptions = {});

    VectorDataSet() = default;
    explicit VectorDataSet(GDALDataset* ptr) noexcept;

    VectorDataSet(VectorDataSet&&);
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

    GDALDataset* get() const;
    VectorDriver driver();

private:
    GDALDataset* _ptr = nullptr;
};

class RasterDriver
{
public:
    static bool is_supported(RasterType);
    static RasterDriver create(RasterType);
    static RasterDriver create(const fs::path& filename);

    explicit RasterDriver(GDALDriver& driver);

    template <typename T>
    RasterDataSet create_dataset(int32_t rows, int32_t cols, int32_t numBands, const fs::path& filename, gsl::span<const std::string> driverOptions = {})
    {
        auto options = create_string_list(driverOptions);
        return RasterDataSet(check_pointer(_driver.Create(filename.string().c_str(), cols, rows, numBands, TypeResolve<T>::value, options.List()), "Failed to create data set"));
    }

    RasterDataSet create_dataset(int32_t rows, int32_t cols, int32_t numBands, const fs::path& filename, const std::type_info& dataType);

    // Use for the memory driver, when there is no path
    template <typename T>
    RasterDataSet create_dataset(int32_t rows, int32_t cols, int32_t numBands, gsl::span<const std::string> driverOptions = {})
    {
        auto options = create_string_list(driverOptions);
        return RasterDataSet(check_pointer(_driver.Create("", cols, rows, numBands, TypeResolve<T>::value, options.List()), "Failed to create data set"));
    }

    // Use for the memory driver, when there is no path
    RasterDataSet create_dataset(int32_t rows, int32_t cols, int32_t numBands, const std::type_info& dataType);

    RasterDataSet create_dataset_copy(const RasterDataSet& reference, const fs::path& filename, gsl::span<const std::string> driverOptions = {});

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
    VectorDataSet create_dataset_copy(const VectorDataSet& reference, const fs::path& filename, const std::vector<std::string>& driverOptions = {});

    VectorType type() const;

private:
    GDALDriver& _driver;
};

FileType detect_file_type(const fs::path& path);

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
