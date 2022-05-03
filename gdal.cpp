#include "infra/gdal.h"
#include "infra/cast.h"
#include "infra/environmentvariable.h"
#include "infra/exception.h"
#include "infra/filesystem.h"
#include "infra/gdallog.h"
#include "infra/scopeguard.h"
#include "infra/string.h"

#ifdef EMBED_GDAL_DATA
#include "embedgdaldata.h"
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <gdal_version.h>
#include <ogrsf_frmts.h>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace inf::gdal {

using namespace std::string_literals;

namespace {

static const std::unordered_map<RasterType, const char*>
    s_rasterDriverLookup{{
        {RasterType::Memory, "MEM"},
        {RasterType::ArcAscii, "AAIGrid"},
        {RasterType::GeoTiff, "GTiff"},
        {RasterType::Gif, "GIF"},
        {RasterType::Png, "PNG"},
        {RasterType::PcRaster, "PCRaster"},
        {RasterType::Netcdf, "netCDF"},
        {RasterType::TileDB, "TileDB"},
        {RasterType::MBTiles, "MBTiles"},
        {RasterType::Grib, "GRIB"},
        {RasterType::Vrt, "VRT"},
    }};

static const std::unordered_map<std::string, RasterType> s_rasterDriverDescLookup{{
    {"MEM", RasterType::Memory},
    {"AAIGrid", RasterType::ArcAscii},
    {"GTiff", RasterType::GeoTiff},
    {"GIF", RasterType::Gif},
    {"PNG", RasterType::Png},
    {"PCRaster", RasterType::PcRaster},
    {"netCDF", RasterType::Netcdf},
    {"TileDB", RasterType::TileDB},
    {"MBTiles", RasterType::MBTiles},
    {"GRIB", RasterType::Grib},
    {"VRT", RasterType::Vrt},
}};

static const std::unordered_map<VectorType, const char*> s_vectorDriverLookup{{
    {VectorType::Memory, "Memory"},
    {VectorType::Csv, "CSV"},
    {VectorType::Tab, "CSV"},
    {VectorType::ShapeFile, "ESRI Shapefile"},
    {VectorType::Xlsx, "XLSX"},
    {VectorType::GeoJson, "GeoJSON"},
    {VectorType::GeoPackage, "GPKG"},
    {VectorType::Vrt, "OGR_VRT"},
}};

static const std::unordered_map<std::string, VectorType> s_vectorDriverDescLookup{{
    {"Memory", VectorType::Memory},
    {"CSV", VectorType::Csv},
    {"CSV", VectorType::Tab},
    {"ESRI Shapefile", VectorType::ShapeFile},
    {"XLSX", VectorType::Xlsx},
    {"GeoJSON", VectorType::GeoJson},
    {"GPKG", VectorType::GeoPackage},
    {"OGR_VRT", VectorType::Vrt},
}};

static std::string get_extension_lowercase(const fs::path& filepath)
{
    return str::lowercase(filepath.extension().string());
}

template <typename Unsigned, typename Signed>
Unsigned signedToUnsigned(Signed value, const char* valueDesc)
{
    if (value < 0) {
        throw RuntimeError("Unexpected negative value for: {}", valueDesc);
    }

    return static_cast<Unsigned>(value);
}

} // namespace

Registration::Registration()
: Registration(RegistrationConfig())
{
}

Registration::Registration(RegistrationConfig cfg)
{
    register_gdal(cfg);
}

Registration::~Registration()
{
    unregister_gdal();
}

EmbeddedDataRegistration::EmbeddedDataRegistration()
{
#ifdef EMBED_GDAL_DATA
    create_embedded_data();
#endif
}

EmbeddedDataRegistration::~EmbeddedDataRegistration()
{
#ifdef EMBED_GDAL_DATA
    destroy_embedded_data();
#endif
}

void register_gdal()
{
    register_gdal(RegistrationConfig());
}

void register_gdal(RegistrationConfig cfg)
{
#ifdef EMBED_GDAL_DATA
    create_embedded_data();
#endif

    register_embedded_data(cfg.projdbPath);

#ifdef __EMSCRIPTEN__
    GDALRegister_MEM();
    GDALRegister_PNG();
    GDALRegister_GTiff();
    GDALRegister_AAIGrid();
#else
    GDALAllRegister();
#endif

    if (cfg.setLogHandler) {
        set_log_handler();
    }
}

void unregister_gdal()
{
    unregister_embedded_data();

#ifdef EMBED_GDAL_DATA
    destroy_embedded_data();
#endif

    GDALDestroy();
}

void register_embedded_data()
{
#ifdef EMBED_GDAL_DATA
    register_embedded_data_file_finder();
#endif
}

void register_embedded_data(const fs::path& p)
{
    register_embedded_data();

    if (!p.empty()) {
#if GDAL_VERSION_MAJOR > 2
        const std::string path           = str::from_u8(p.u8string());
        std::array<const char*, 2> paths = {
            path.c_str(),
            nullptr,
        };

        OSRSetPROJSearchPaths(paths.data());
#endif

        // Also set the environment variable
        // e.g. Spatialite library does not use gdal settings
        env::set("PROJ_LIB", p.u8string());
    }
}

void unregister_embedded_data()
{
#ifdef EMBED_GDAL_DATA
    unregister_embedded_data_file_finder();
#endif
}

std::string get_memory_file_buffer(const fs::path& p, bool remove)
{
    std::string result;
    vsi_l_offset length;
    auto* data = VSIGetMemFileBuffer(str::from_u8(p.u8string()).c_str(), &length, remove ? TRUE : FALSE);
    ScopeGuard guard([=]() {
        if (remove) {
            CPLFree(data);
        }
    });

    if (data) {
        result.assign(reinterpret_cast<char*>(data), length);
    }

    return result;
}

bool RasterDriver::is_supported(RasterType type)
{
    if (type == RasterType::Unknown) {
        throw InvalidArgument("Invalid raster type specified");
    }

    return nullptr != GetGDALDriverManager()->GetDriverByName(s_rasterDriverLookup.at(type));
}

RasterDriver RasterDriver::create(RasterType type)
{
    if (type == RasterType::Unknown) {
        throw InvalidArgument("Invalid raster type specified");
    }

    auto driverName = s_rasterDriverLookup.at(type);
    auto* driverPtr = GetGDALDriverManager()->GetDriverByName(driverName);
    if (driverPtr == nullptr) {
        throw RuntimeError("Failed to get raster driver: {}", driverName);
    }

    return RasterDriver(*driverPtr);
}

RasterDriver RasterDriver::create(const fs::path& filename)
{
    auto rasterType = guess_rastertype_from_filename(filename);
    if (rasterType != RasterType::Unknown) {
        return create(rasterType);
    }

    throw RuntimeError("Failed to determine raster type from filename: {}", filename);
}

RasterDriver::RasterDriver(GDALDriver& driver)
: _driver(driver)
{
}

RasterDataSet RasterDriver::create_dataset(int32_t rows, int32_t cols, int32_t numBands, const fs::path& filename, const std::type_info& dataType)
{
    return RasterDataSet(check_pointer(_driver.Create(str::from_u8(filename.u8string()).c_str(), cols, rows, numBands, resolve_type(dataType), nullptr), "Failed to create data set"));
}

RasterDataSet RasterDriver::create_dataset(int32_t rows, int32_t cols, int32_t numBands, const std::type_info& dataType)
{
    return RasterDataSet(check_pointer(_driver.Create("", cols, rows, numBands, resolve_type(dataType), nullptr), "Failed to create data set"));
}

RasterDataSet RasterDriver::create_dataset_copy(const RasterDataSet& reference, const fs::path& filename, std::span<const std::string> driverOptions)
{
    auto options = create_string_list(driverOptions);
    return RasterDataSet(check_pointer(_driver.CreateCopy(
                                           str::from_u8(filename.u8string()).c_str(),
                                           reference.get(),
                                           FALSE,
                                           options.List(),
                                           nullptr,
                                           nullptr),
                                       "Failed to create data set copy"));
}

RasterType RasterDriver::type() const
{
    try {
        return s_rasterDriverDescLookup.at(_driver.GetDescription());
    } catch (const std::out_of_range&) {
        throw RuntimeError("Failed to determine raster type for driver: {}", _driver.GetDescription());
    }
}

bool VectorDriver::is_supported(VectorType type)
{
    if (type == VectorType::Unknown) {
        throw InvalidArgument("Invalid vector type specified");
    }

    return nullptr != GetGDALDriverManager()->GetDriverByName(s_vectorDriverLookup.at(type));
}

VectorDriver VectorDriver::create(const fs::path& filename)
{
    auto vectorType = guess_vectortype_from_filename(filename.u8string());
    if (vectorType != VectorType::Unknown) {
        return create(vectorType);
    }

    throw RuntimeError("Failed to determine vector type from filename: {}", filename);
}

VectorDriver VectorDriver::create(VectorType type)
{
    if (type == VectorType::Unknown) {
        throw InvalidArgument("Invalid vector type specified");
    }

    auto driverName = s_vectorDriverLookup.at(type);
    auto* driverPtr = GetGDALDriverManager()->GetDriverByName(driverName);
    if (driverPtr == nullptr) {
        throw RuntimeError("Failed to get vector driver: {}", driverName);
    }

    return VectorDriver(*driverPtr);
}

VectorDriver::VectorDriver(GDALDriver& driver)
: _driver(driver)
{
}

VectorDataSet VectorDriver::create_dataset()
{
    return create_dataset("");
}

VectorDataSet VectorDriver::create_dataset(const fs::path& filename, const std::vector<std::string>& creationOptions)
{
    auto options = create_string_list(creationOptions);
    return VectorDataSet(check_pointer(_driver.Create(str::from_u8(filename.u8string()).c_str(), 0, 0, 0, GDT_Unknown, options), "Failed to create vector data set"));
}

VectorType VectorDriver::type() const
{
    try {
        return s_vectorDriverDescLookup.at(_driver.GetDescription());
    } catch (const std::out_of_range&) {
        throw RuntimeError("Failed to determine vector type for driver: {}", _driver.GetDescription());
    }
}

RasterBand::RasterBand(GDALRasterBand* ptr)
: _band(ptr)
{
}

GDALRasterBand* RasterBand::get()
{
    return _band;
}

const GDALRasterBand* RasterBand::get() const
{
    return _band;
}

static GDALDataset* create_data_set(const fs::path& filePath,
                                    unsigned int openFlags,
                                    const char* const* drivers,
                                    const std::vector<std::string>& driverOpts)
{
    // use generic_u8string otherwise the path contains backslashes on windows
    // In memory file paths like /vsimem/file.asc in memory will then be \\vsimem\\file.asc
    // which is not recognized by gdal
    const auto path = str::from_u8(filePath.generic_u8string());
    auto options    = create_string_list(driverOpts);
    return reinterpret_cast<GDALDataset*>(GDALOpenEx(
        path.c_str(),
        openFlags,
        drivers,
        options.List(),
        nullptr));
}

static uint32_t open_mode_to_gdal_mode(OpenMode mode)
{
    return mode == OpenMode::ReadOnly ? GDAL_OF_READONLY : GDAL_OF_UPDATE;
}

static std::string open_mode_to_string(OpenMode mode)
{
    return mode == OpenMode::ReadOnly ? "reading" : "writing";
}

static RasterDataSet raster_open_impl(OpenMode mode, const fs::path& filePath, const std::vector<std::string>& driverOpts)
{
    auto* dataSet = create_data_set(filePath, open_mode_to_gdal_mode(mode) | GDAL_OF_RASTER, nullptr, driverOpts);
    if (!dataSet) {
        throw RuntimeError("Failed to open raster file '{}' for {}", filePath, open_mode_to_string(mode));
    }

    return RasterDataSet(dataSet);
}

static RasterDataSet raster_open_impl(OpenMode mode, const fs::path& filePath, RasterType type, const std::vector<std::string>& driverOpts)
{
    if (type == RasterType::Unknown) {
        type = guess_rastertype_from_filename(filePath);
        if (type == RasterType::Unknown) {
            throw RuntimeError("Failed to determine raster type for file ('{}')", filePath);
        }
    }

    std::array<const char*, 2> allowedDrivers{{
        s_rasterDriverLookup.at(type),
        nullptr,
    }};

    return RasterDataSet(check_pointer_msg_cb(create_data_set(filePath,
                                                              open_mode_to_gdal_mode(mode) | GDAL_OF_RASTER,
                                                              allowedDrivers.data(),
                                                              driverOpts),
                                              [&]() { return fmt::format("Failed to open raster file: {}", filePath); }));
}

RasterDataSet RasterDataSet::create(const fs::path& filePath, const std::vector<std::string>& driverOpts)
{
    return open(filePath, driverOpts);
}

RasterDataSet RasterDataSet::create(const fs::path& filePath, RasterType type, const std::vector<std::string>& driverOpts)
{
    return open(filePath, type, driverOpts);
}

RasterDataSet RasterDataSet::open(const fs::path& filePath, const std::vector<std::string>& driverOpts)
{
    return raster_open_impl(OpenMode::ReadOnly, filePath, driverOpts);
}

RasterDataSet RasterDataSet::open(const fs::path& filePath, RasterType type, const std::vector<std::string>& driverOpts)
{
    return raster_open_impl(OpenMode::ReadOnly, filePath, type, driverOpts);
}

RasterDataSet RasterDataSet::open_for_writing(const fs::path& filePath, const std::vector<std::string>& driverOpts)
{
    return raster_open_impl(OpenMode::ReadWrite, filePath, driverOpts);
}

RasterDataSet RasterDataSet::open_for_writing(const fs::path& filePath, RasterType type, const std::vector<std::string>& driverOpts)
{
    return raster_open_impl(OpenMode::ReadWrite, filePath, type, driverOpts);
}

RasterDataSet::RasterDataSet(GDALDataset* ptr) noexcept
: _ptr(ptr)
{
}

RasterDataSet::RasterDataSet(RasterDataSet&& rhs) noexcept
: _ptr(rhs._ptr)
{
    rhs._ptr = nullptr;
}

RasterDataSet::~RasterDataSet() noexcept
{
    GDALClose(reinterpret_cast<GDALDatasetH>(_ptr));
}

RasterDataSet& RasterDataSet::operator=(RasterDataSet&& rhs) noexcept
{
    if (_ptr) {
        GDALClose(reinterpret_cast<GDALDatasetH>(_ptr));
    }

    _ptr     = rhs._ptr;
    rhs._ptr = nullptr;
    return *this;
}

bool RasterDataSet::is_valid() const noexcept
{
    return _ptr != nullptr;
}

int32_t RasterDataSet::raster_count() const
{
    assert(_ptr);
    return _ptr->GetRasterCount();
}

int32_t RasterDataSet::x_size() const
{
    assert(_ptr);
    return _ptr->GetRasterXSize();
}

int32_t RasterDataSet::y_size() const
{
    assert(_ptr);
    return _ptr->GetRasterYSize();
}

bool RasterDataSet::has_valid_geotransform() const
{
    assert(_ptr);
    std::array<double, 6> trans;
    return _ptr->GetGeoTransform(trans.data()) == CE_None;
}

std::array<double, 6> RasterDataSet::geotransform() const
{
    assert(_ptr);
    std::array<double, 6> trans;
    check_error(_ptr->GetGeoTransform(trans.data()), "Failed to get extent metadata");
    return trans;
}

void RasterDataSet::set_geotransform(const std::array<double, 6>& trans)
{
    assert(_ptr);
    check_error(_ptr->SetGeoTransform(const_cast<double*>(trans.data())), "Failed to set geo transform");
}

std::optional<double> RasterDataSet::nodata_value(int bandNr) const
{
    assert(bandNr > 0);

    auto* band = _ptr->GetRasterBand(bandNr);
    if (band == nullptr) {
        throw RuntimeError("Invalid dataset band number: {}", bandNr);
    }

    int success = 0;
    auto value  = band->GetNoDataValue(&success);
    if (success) {
        return std::make_optional(value);
    }

    return std::optional<double>();
}

void RasterDataSet::set_nodata_value(int bandNr, std::optional<double> value) const
{
    assert(bandNr > 0);

    if (driver().type() == RasterType::TileDB) {
        return;
    }

    auto* band = _ptr->GetRasterBand(bandNr);
    if (band == nullptr) {
        throw RuntimeError("Invalid dataset band number: {}", bandNr);
    }

    if (value) {
        check_error(band->SetNoDataValue(*value), "Failed to set nodata value");
    } else {
        if (auto err = band->DeleteNoDataValue(); err != CE_None) {
            if (err == CE_Failure && CPLGetLastErrorNo() == CPLE_NotSupported) {
                // not supported by the driver
                return;
            }

            check_error(err, "Failed to delete nodata value");
        }
    }
}

void RasterDataSet::set_colortable(int bandNr, const GDALColorTable* ct)
{
    assert(_ptr);
    assert(bandNr > 0);
    auto* band = check_pointer(_ptr->GetRasterBand(bandNr), "Failed to get raster band");
    check_error(band->SetColorTable(const_cast<GDALColorTable*>(ct)), "Failed to set color table");
}

std::string RasterDataSet::projection() const
{
    assert(_ptr);
    return _ptr->GetProjectionRef();
}

void RasterDataSet::set_projection(const std::string& proj)
{
    assert(_ptr);
    if (!proj.empty()) {
        check_error(_ptr->SetProjection(proj.c_str()), "Failed to set projection");
    }
}

std::string RasterDataSet::metadata_item(const std::string& name, const std::string& domain)
{
    std::string result;

    if (auto* value = _ptr->GetMetadataItem(name.c_str(), domain.c_str()); value != nullptr) {
        result.assign(value);
    }

    return result;
}

std::string RasterDataSet::band_metadata_item(int bandNr, const std::string& name, const std::string& domain)
{
    std::string result;

    assert(bandNr > 0);
    auto* band = check_pointer(_ptr->GetRasterBand(bandNr), "Failed to get raster band");
    if (auto* value = band->GetMetadataItem(name.c_str(), domain.c_str()); value != nullptr) {
        result.assign(value);
    }

    return result;
}

std::unordered_map<std::string, std::string> RasterDataSet::metadata(const std::string& domain)
{
    std::unordered_map<std::string, std::string> result;

    char** data = _ptr->GetMetadata(domain.c_str());
    if (data != nullptr) {
        int index = 0;
        while (data[index] != nullptr) {
            auto keyValue = str::split_view(data[index++], '=');
            if (keyValue.size() == 2) {
                result.emplace(keyValue[0], keyValue[1]);
            }
        }
    }

    return result;
}

void RasterDataSet::set_metadata(const std::string& name, const std::string& value, const std::string& domain)
{
    check_error(_ptr->SetMetadataItem(name.c_str(), value.c_str(), domain.c_str()), "Failed to set metadata");
}

void RasterDataSet::set_band_metadata(int bandNr, const std::string& name, const std::string& value, const std::string& domain)
{
    assert(bandNr > 0);
    auto* band = check_pointer(_ptr->GetRasterBand(bandNr), "Failed to get raster band");
    check_error(band->SetMetadataItem(name.c_str(), value.c_str(), domain.c_str()), "Failed to set band metadata");
}

void RasterDataSet::set_band_description(int bandNr, const std::string& description)
{
    assert(bandNr > 0);
    auto* band = check_pointer(_ptr->GetRasterBand(bandNr), "Failed to get raster band");
    band->SetDescription(description.c_str());
}

std::vector<std::string> RasterDataSet::metadata_domains() const noexcept
{
    std::vector<std::string> result;

    char** data = _ptr->GetMetadataDomainList();
    if (data) {
        int index = 0;
        while (data[index] != nullptr) {
            result.push_back(data[index++]);
        }

        CSLDestroy(data);
    }

    return result;
}

RasterBand RasterDataSet::rasterband(int bandNr) const
{
    assert(bandNr > 0);
    return RasterBand(check_pointer(_ptr->GetRasterBand(bandNr), "Invalid band index"));
}

RasterType RasterDataSet::type() const
{
    return driver().type();
}

const std::type_info& RasterDataSet::band_datatype(int bandNr) const
{
    assert(_ptr);
    assert(bandNr > 0);
    return resolve_type(check_pointer(_ptr->GetRasterBand(bandNr), "Invalid band index")->GetRasterDataType());
}

void RasterDataSet::read_rasterdata(int band, int xOff, int yOff, int xSize, int ySize, const std::type_info& type, void* pData, int bufXSize, int bufYSize, int pixelSize, int lineSize) const
{
    auto* bandPtr = _ptr->GetRasterBand(band);
    check_error(bandPtr->RasterIO(GF_Read, xOff, yOff, xSize, ySize, pData, bufXSize, bufYSize, resolve_type(type), pixelSize, lineSize), "Failed to read raster data");
}

void RasterDataSet::write_rasterdata(int band, int xOff, int yOff, int xSize, int ySize, const std::type_info& type, const void* pData, int bufXSize, int bufYSize) const
{
    auto* bandPtr = _ptr->GetRasterBand(band);
    check_error(bandPtr->RasterIO(GF_Write, xOff, yOff, xSize, ySize, const_cast<void*>(pData), bufXSize, bufYSize, resolve_type(type), 0, 0), "Failed to write raster data");
}

void RasterDataSet::write_geometadata(const GeoMetadata& meta)
{
    set_geotransform(metadata_to_geo_transform(meta));
    set_projection(meta.projection);

    if (raster_count() > 0) {
        set_nodata_value(1, meta.nodata);
    }
}

GeoMetadata RasterDataSet::geometadata() const
{
    if (raster_count() != 1) {
        throw RuntimeError("Multiple raster bands present, specify the band number");
    }

    return geometadata(1);
}

GeoMetadata RasterDataSet::geometadata(int bandNr) const
{
    GeoMetadata meta;

    meta.cols       = x_size();
    meta.rows       = y_size();
    meta.nodata     = nodata_value(bandNr);
    meta.projection = projection();
    fill_geometadata_from_geo_transform(meta, geotransform());

    return meta;
}

void RasterDataSet::flush_cache()
{
    _ptr->FlushCache();
}

void RasterDataSet::add_band(GDALDataType type, const void* data)
{
    // convert the data pointer to a string
    std::array<char, 32> buf;
    auto writtenCharacters = CPLPrintPointer(buf.data(), const_cast<void*>(data), truncate<int>(buf.size()));
    buf[writtenCharacters] = 0;

    auto pointerString = fmt::format("DATAPOINTER={}", buf.data());
    std::array<const char*, 2> options{{pointerString.c_str(), nullptr}};

    _ptr->AddBand(type, const_cast<char**>(options.data()));
}

GDALDataset* RasterDataSet::get() const
{
    return _ptr;
}

RasterDriver RasterDataSet::driver()
{
    return RasterDriver(*_ptr->GetDriver());
}

RasterDriver RasterDataSet::driver() const
{
    return RasterDriver(*_ptr->GetDriver());
}

VectorDataSet VectorDataSet::create(const fs::path& filePath, const std::vector<std::string>& driverOptions)
{
    return open(filePath, driverOptions);
}

VectorDataSet VectorDataSet::create(const fs::path& filePath, VectorType type, const std::vector<std::string>& driverOptions)
{
    return open(filePath, type, driverOptions);
}

static VectorDataSet open_vector_impl(OpenMode mode, const fs::path& filePath, const std::vector<std::string>& driverOptions)
{
    auto* dsPtr = create_data_set(filePath, open_mode_to_gdal_mode(mode) | GDAL_OF_VECTOR, nullptr, driverOptions);
    if (!dsPtr) {
        throw RuntimeError("Failed to open vector file '{}'", filePath);
    }

    return VectorDataSet(dsPtr);
}

static VectorDataSet open_vector_impl(OpenMode mode, const fs::path& filePath, VectorType type, const std::vector<std::string>& driverOptions)
{
    if (type == VectorType::Unknown) {
        type = guess_vectortype_from_filename(filePath);
        if (type == VectorType::Unknown) {
            throw RuntimeError("Failed to determine vector type for file ('{}')", filePath);
        }
    }

    std::array<const char*, 2> drivers{{s_vectorDriverLookup.at(type), nullptr}};

    auto* dsPtr = create_data_set(filePath, open_mode_to_gdal_mode(mode) | GDAL_OF_VECTOR, drivers.data(), driverOptions);
    if (!dsPtr) {
        throw RuntimeError("Failed to open vector file '{}'", filePath);
    }

    return VectorDataSet(dsPtr);
}

VectorDataSet VectorDataSet::open(const fs::path& filePath, const std::vector<std::string>& driverOptions)
{
    return open_vector_impl(OpenMode::ReadOnly, filePath, driverOptions);
}

VectorDataSet VectorDataSet::open(const fs::path& filePath, VectorType type, const std::vector<std::string>& driverOptions)
{
    return open_vector_impl(OpenMode::ReadOnly, filePath, type, driverOptions);
}

VectorDataSet VectorDataSet::open_for_writing(const fs::path& filePath, const std::vector<std::string>& driverOptions)
{
    return open_vector_impl(OpenMode::ReadWrite, filePath, driverOptions);
}

VectorDataSet VectorDataSet::open_for_writing(const fs::path& filePath, VectorType type, const std::vector<std::string>& driverOptions)
{
    return open_vector_impl(OpenMode::ReadWrite, filePath, type, driverOptions);
}

VectorDataSet::VectorDataSet(GDALDataset* ptr) noexcept
: _ptr(ptr)
{
}

VectorDataSet::VectorDataSet(VectorDataSet&& rhs) noexcept
: _ptr(rhs._ptr)
{
    rhs._ptr = nullptr;
}

VectorDataSet::~VectorDataSet() noexcept
{
    GDALClose(reinterpret_cast<GDALDatasetH>(_ptr));
}

VectorDataSet& VectorDataSet::operator=(VectorDataSet&& rhs)
{
    if (_ptr) {
        GDALClose(reinterpret_cast<GDALDatasetH>(_ptr));
    }

    _ptr     = rhs._ptr;
    rhs._ptr = nullptr;
    return *this;
}

bool VectorDataSet::is_valid() const
{
    return _ptr != nullptr;
}

int32_t VectorDataSet::layer_count() const
{
    assert(_ptr);
    return _ptr->GetLayerCount();
}

std::string VectorDataSet::projection() const
{
    assert(_ptr);
    return _ptr->GetProjectionRef();
}

void VectorDataSet::set_projection(const std::string& proj)
{
    assert(_ptr);
    if (!proj.empty()) {
        check_error(_ptr->SetProjection(proj.c_str()), "Failed to set projection");
    }
}

void VectorDataSet::set_metadata(const std::string& name, const std::string& value, const std::string& domain)
{
    check_error(_ptr->SetMetadataItem(name.c_str(), value.c_str(), domain.c_str()), "Failed to set metadata");
}

Layer VectorDataSet::layer(int index)
{
    assert(_ptr);
    return Layer(check_pointer(_ptr->GetLayer(index), "Invalid layer index"));
}

Layer VectorDataSet::layer(const std::string& name)
{
    assert(_ptr);
    return Layer(check_pointer(_ptr->GetLayerByName(name.c_str()), "Invalid layer name"));
}

Layer VectorDataSet::create_layer(const std::string& name, const std::vector<std::string>& driverOptions)
{
    return create_layer(name, Geometry::Type::Unknown, driverOptions);
}

static OGRwkbGeometryType to_gdal_type(Geometry::Type type)
{
    switch (type) {
    case Geometry::Type::Point:
        return wkbPoint;
    case Geometry::Type::Collection:
        return wkbGeometryCollection;
    case Geometry::Type::Line:
        return wkbLineString;
    case Geometry::Type::MultiLine:
        return wkbMultiLineString;
    case Geometry::Type::Polygon:
        return wkbPolygon;
    case Geometry::Type::MultiPolygon:
        return wkbMultiPolygon;
    case Geometry::Type::Unknown:
    default:
        return wkbUnknown;
    }
}

Layer VectorDataSet::create_layer(const std::string& name, Geometry::Type type, const std::vector<std::string>& driverOptions)
{
    assert(_ptr);
    auto options = create_string_list(driverOptions);
    return Layer(check_pointer(_ptr->CreateLayer(name.c_str(), nullptr, to_gdal_type(type), options.List()), "Layer creation failed"));
}

Layer VectorDataSet::copy_layer(Layer srcLayer, const char* newLayerName, const std::vector<std::string>& driverOptions)
{
    auto options = create_string_list(driverOptions);
    return Layer(check_pointer(_ptr->CopyLayer(srcLayer.get(), newLayerName, options.List()), "Layer copy failed"));
}

Layer VectorDataSet::create_layer(const std::string& name, SpatialReference& spatialRef, Geometry::Type type, const std::vector<std::string>& driverOptions)
{
    assert(_ptr);
    auto options = create_string_list(driverOptions);
    return Layer(check_pointer(_ptr->CreateLayer(name.c_str(), spatialRef.get(), to_gdal_type(type), options.List()), "Layer creation failed"));
}

GDALDataset* VectorDataSet::get() const
{
    return _ptr;
}

VectorDriver VectorDataSet::driver()
{
    return VectorDriver(*_ptr->GetDriver());
}

RasterType guess_rastertype_from_filename(const fs::path& filePath)
{
    auto ext = get_extension_lowercase(filePath);
    if (ext == ".asc") {
        return RasterType::ArcAscii;
    } else if (ext == ".tiff" || ext == ".tif") {
        return RasterType::GeoTiff;
    } else if (ext == ".gif") {
        return RasterType::Gif;
    } else if (ext == ".png") {
        return RasterType::Png;
    } else if (ext == ".map") {
        return RasterType::PcRaster;
    } else if (ext == ".nc") {
        return RasterType::Netcdf;
    } else if (ext == ".mbtiles") {
        return RasterType::MBTiles;
    } else if (ext == ".grib") {
        return RasterType::Grib;
    } else if (ext == ".vrt") {
        return RasterType::Vrt;
    }

    return RasterType::Unknown;
}

VectorType guess_vectortype_from_filename(const fs::path& filePath)
{
    auto ext = get_extension_lowercase(filePath);
    if (ext == ".csv") {
        return VectorType::Csv;
    } else if (ext == ".tab") {
        return VectorType::Tab;
    } else if (ext == ".shp" || ext == ".dbf") {
        return VectorType::ShapeFile;
    } else if (ext == ".xlsx") {
        return VectorType::Xlsx;
    } else if (ext == ".json" || ext == ".geojson") {
        return VectorType::GeoJson;
    } else if (ext == ".gpkg") {
        return VectorType::GeoPackage;
    } else if (ext == ".vrt") {
        return VectorType::Vrt;
    }

    return VectorType::Unknown;
}

FileType detect_file_type(const fs::path& path)
{
    if (auto ds = RasterDataSet(create_data_set(path, GDAL_OF_READONLY | GDAL_OF_RASTER, nullptr, {})); ds.is_valid()) {
        return FileType::Raster;
    } else if (auto ds = VectorDataSet(create_data_set(path, GDAL_OF_READONLY | GDAL_OF_VECTOR, nullptr, {})); ds.is_valid()) {
        return FileType::Vector;
    }

    return FileType::Unknown;
}

void fill_geometadata_from_geo_transform(GeoMetadata& meta, const std::array<double, 6>& geoTrans)
{
    if (geoTrans[2] == 0.0 && geoTrans[4] == 0.0) {
        meta.cellSize = GeoMetadata::CellSize(geoTrans[1], geoTrans[5]);
    }

    // transform the lower left coordinate (0.0, meta.rows)
    meta.xll = geoTrans[0] + geoTrans[1] * 0.0 + geoTrans[2] * meta.rows;
    meta.yll = geoTrans[3] + geoTrans[4] * 0.0 + geoTrans[5] * meta.rows;
}

MemoryFile::MemoryFile(const char* path, std::span<const uint8_t> dataBuffer)
: MemoryFile(std::string(path), dataBuffer)
{
}

MemoryFile::MemoryFile(const char* path, std::string_view dataBuffer)
: MemoryFile(std::string(path), dataBuffer)
{
}

MemoryFile::MemoryFile(std::string path, std::span<const uint8_t> dataBuffer)
: _path(std::move(path))
, _ptr(VSIFileFromMemBuffer(_path.c_str(),
                            const_cast<GByte*>(reinterpret_cast<const GByte*>(dataBuffer.data())),
                            dataBuffer.size(), FALSE /*no ownership*/))
{
}

MemoryFile::MemoryFile(std::string path, std::string_view dataBuffer)
: MemoryFile(path, std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(dataBuffer.data()), dataBuffer.size()))
{
}

MemoryFile::MemoryFile(const fs::path& path, std::span<const uint8_t> dataBuffer)
: MemoryFile(str::from_u8(path.u8string()), dataBuffer)
{
}

MemoryFile::MemoryFile(const fs::path& path, std::string_view dataBuffer)
: MemoryFile(str::from_u8(path.u8string()), dataBuffer)
{
}

MemoryFile::MemoryFile(MemoryFile&& other) noexcept
: _path(std::move(other._path))
, _ptr(other._ptr)
{
    other._ptr = nullptr;
}

MemoryFile::~MemoryFile() noexcept
{
    close();
}

MemoryFile& MemoryFile::operator=(MemoryFile&& other) noexcept
{
    close();
    _path      = std::move(other._path);
    _ptr       = other._ptr;
    other._ptr = nullptr;

    return *this;
}

const std::string& MemoryFile::path() const
{
    return _path;
}

std::span<uint8_t> MemoryFile::data()
{
    vsi_l_offset dataSize = 0;
    auto* data            = VSIGetMemFileBuffer(_path.c_str(), &dataSize, FALSE /*do not take ownership*/);

    return std::span<uint8_t>(data, dataSize);
}

void MemoryFile::close() noexcept
{
    if (_ptr) {
        VSIFCloseL(_ptr);
        _ptr = nullptr;
    }
}

std::string read_memory_file_as_text(const fs::path& path, MemoryReadMode mode)
{
    vsi_l_offset dataLength = 0;
    CplPointer<GByte> dataPtr(VSIGetMemFileBuffer(str::from_u8(path.generic_u8string()).c_str(), &dataLength, mode == MemoryReadMode::StealContents ? TRUE : FALSE));
    if (!dataPtr) {
        throw RuntimeError("Invalid memory file: {}", path);
    }

    return std::string(reinterpret_cast<char*>(dataPtr.get()), dataLength);
}
}
