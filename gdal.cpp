#include "infra/gdal.h"
#include "infra/cast.h"
#include "infra/exception.h"
#include "infra/filesystem.h"
#include "infra/gdallog.h"
#include "infra/string.h"

#ifdef EMBED_GDAL_DATA
#include "embedgdaldata.h"
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <ogrsf_frmts.h>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace inf::gdal {

using namespace std::string_literals;

namespace {

template <typename T>
class CplPointer
{
public:
    CplPointer() = default;
    CplPointer(T* ptr)
    : _ptr(ptr)
    {
    }

    ~CplPointer()
    {
        CPLFree(_ptr);
    }

    T** ptrAddress()
    {
        return &_ptr;
    }

    operator T*()
    {
        return _ptr;
    }

private:
    T* _ptr = nullptr;
};

static const std::unordered_map<RasterType, const char*>
    s_rasterDriverLookup{{
        {RasterType::Memory, "MEM"},
        {RasterType::ArcAscii, "AAIGrid"},
        {RasterType::GeoTiff, "GTiff"},
        {RasterType::Gif, "GIF"},
        {RasterType::Png, "PNG"},
        {RasterType::PcRaster, "PCRaster"},
        {RasterType::Netcdf, "netCDF"},
    }};

static const std::unordered_map<std::string, RasterType> s_rasterDriverDescLookup{{
    {"MEM", RasterType::Memory},
    {"AAIGrid", RasterType::ArcAscii},
    {"GTiff", RasterType::GeoTiff},
    {"GIF", RasterType::Gif},
    {"PNG", RasterType::Png},
    {"PCRaster", RasterType::PcRaster},
    {"netCDF", RasterType::Netcdf},
}};

static const std::unordered_map<VectorType, const char*> s_vectorDriverLookup{{
    {VectorType::Memory, "Memory"},
    {VectorType::Csv, "CSV"},
    {VectorType::Tab, "CSV"},
    {VectorType::ShapeFile, "ESRI Shapefile"},
    {VectorType::Xlsx, "XLSX"},
    {VectorType::GeoJson, "GeoJSON"},
}};

static const std::unordered_map<std::string, VectorType> s_vectorDriverDescLookup{{
    {"Memory", VectorType::Memory},
    {"CSV", VectorType::Csv},
    {"CSV", VectorType::Tab},
    {"ESRI Shapefile", VectorType::ShapeFile},
    {"XLSX", VectorType::Xlsx},
    {"GeoJSON", VectorType::GeoJson},
}};

static std::string getExtenstion(const fs::path& filepath)
{
    return filepath.extension().string();
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

SpatialReference::SpatialReference()
: _srs(std::make_unique<OGRSpatialReference>())
{
}

SpatialReference::SpatialReference(int32_t epsg)
: SpatialReference()
{
    import_from_epsg(epsg);
}

SpatialReference::SpatialReference(const char* wkt)
: SpatialReference()
{
    import_from_wkt(wkt);
}

SpatialReference::SpatialReference(OGRSpatialReference* instance)
: _srs(instance)
{
}

SpatialReference::~SpatialReference() noexcept
{
    // don't simply let the unique_ptr call the destructor, it is deprecated
    OGRSpatialReference::DestroySpatialReference(_srs.release());
}

SpatialReference SpatialReference::clone() const
{
    return SpatialReference(_srs->Clone());
}

SpatialReference SpatialReference::clone_geo_cs() const
{
    return SpatialReference(_srs->CloneGeogCS());
}

void SpatialReference::import_from_epsg(int32_t epsg)
{
    checkError(_srs->importFromEPSG(epsg), "Failed to import spatial reference from epsg");
}

void SpatialReference::import_from_wkt(const char* wkt)
{
    checkError(_srs->importFromWkt(wkt), "Failed to import spatial reference from WKT");
}

std::string SpatialReference::export_to_pretty_wkt() const
{
    CplPointer<char> friendlyWkt;
    checkError(_srs->exportToPrettyWkt(friendlyWkt.ptrAddress(), FALSE), "Failed to export projection to pretty WKT");
    return std::string(friendlyWkt);
}

std::string SpatialReference::export_to_pretty_wkt_simplified() const
{
    CplPointer<char> friendlyWkt;
    checkError(_srs->exportToPrettyWkt(friendlyWkt.ptrAddress(), TRUE), "Failed to export projection to simplified pretty WKT");
    return std::string(friendlyWkt);
}

std::optional<int32_t> SpatialReference::epsg_cs() const
{
    try {
        return str::to_int32(authority_code("PROJCS"));
    } catch (const std::exception&) {
        return std::optional<int32_t>();
    }
}

std::optional<int32_t> SpatialReference::epsg_geog_cs() const
{
    auto epsg = _srs->GetEPSGGeogCS();
    if (epsg == -1) {
        return std::optional<int32_t>();
    }

    return epsg;
}

std::string_view SpatialReference::authority_code(const char* key) const
{
    auto* code = _srs->GetAuthorityCode(key);
    if (code == nullptr) {
        throw RuntimeError("Could not get authority code for '{}'", key);
    }

    return std::string_view(code);
}

void SpatialReference::set_proj_cs(const char* projCs)
{
    _srs->SetProjCS(projCs);
}

void SpatialReference::set_well_known_geog_cs(const char* geogCs)
{
    _srs->SetWellKnownGeogCS(geogCs);
}

void SpatialReference::set_utm(int zone, bool north)
{
    _srs->SetUTM(zone, north ? TRUE : FALSE);
}

OGRSpatialReference* SpatialReference::get() noexcept
{
    return _srs.get();
}

CoordinateTransformer::CoordinateTransformer(SpatialReference source, SpatialReference dest)
: _sourceSRS(std::move(source))
, _targetSRS(std::move(dest))
{
    _transformer.reset(OGRCreateCoordinateTransformation(_sourceSRS.get(), _targetSRS.get()));
    if (!_transformer) {
        throw RuntimeError("Failed to create transformation");
    }
}

CoordinateTransformer::CoordinateTransformer(int32_t sourceEpsg, int32_t destEpsg)
{
    _sourceSRS.import_from_epsg(sourceEpsg);
    _targetSRS.import_from_epsg(destEpsg);

    _transformer.reset(OGRCreateCoordinateTransformation(_sourceSRS.get(), _targetSRS.get()));
    if (!_transformer) {
        throw RuntimeError("Failed to create transformation from EPSG:{} to EPSG:{}", sourceEpsg, destEpsg);
    }
}

Point<double> CoordinateTransformer::transform(const Point<double>& point) const
{
    Point<double> result = point;
    if (!_transformer->Transform(1, &result.x, &result.y)) {
        throw RuntimeError("Failed to transform point ({}, {})", point.x, point.y);
    }

    return result;
}

void CoordinateTransformer::transform_in_place(Point<double>& point) const
{
    if (!_transformer->Transform(1, &point.x, &point.y)) {
        throw RuntimeError("Failed to perform transformation");
    }
}

Coordinate CoordinateTransformer::transform(const Coordinate& coord) const
{
    Coordinate result = coord;
    if (!_transformer->Transform(1, &result.longitude, &result.latitude)) {
        throw RuntimeError("Failed to transform coordinate {}", coord);
    }

    return result;
}

void CoordinateTransformer::transform_in_place(Coordinate& coord) const
{
    if (!_transformer->Transform(1, &coord.longitude, &coord.latitude)) {
        throw RuntimeError("Failed to perform transformation");
    }
}

OGRCoordinateTransformation* CoordinateTransformer::get()
{
    return _transformer.get();
}

Point<double> convert_point_projected(int32_t sourceEpsg, int32_t destEpsg, Point<double> point)
{
    CoordinateTransformer transformer(sourceEpsg, destEpsg);
    return transformer.transform(point);
}

Point<double> projected_to_geographic(int32_t epsg, Point<double> point)
{
    SpatialReference utm(epsg);
    utm.set_proj_cs("UTM 17 / WGS84");
    utm.set_well_known_geog_cs("WGS84");
    utm.set_utm(17);

    auto poLatLong = utm.clone_geo_cs();
    auto trans     = checkPointer(OGRCreateCoordinateTransformation(utm.get(), poLatLong.get()),
        "Failed to create transformation");

    if (!trans->Transform(1, &point.x, &point.y)) {
        throw RuntimeError("Failed to perform transformation");
    }

    return point;
}

std::string projection_to_friendly_name(const std::string& projection)
{
    SpatialReference spatialRef;
    spatialRef.import_from_wkt(projection.c_str());
    return spatialRef.export_to_pretty_wkt_simplified();
}

std::string projection_from_epsg(int32_t epsg)
{
    SpatialReference spatialRef(epsg);
    return spatialRef.export_to_pretty_wkt();
}

std::optional<int32_t> projection_to_geo_epsg(const std::string& projection)
{
    SpatialReference spatialRef(projection.c_str());
    return spatialRef.epsg_geog_cs();
}

std::optional<int32_t> projection_to_epsg(const std::string& projection)
{
    SpatialReference spatialRef(projection.c_str());
    return str::to_int32(spatialRef.authority_code("PROJCS"));
}

Registration::Registration()
{
    registerGdal();
}

Registration::~Registration()
{
    unregisterGdal();
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

void registerGdal()
{
#ifdef EMBED_GDAL_DATA
    create_embedded_data();
    registerEmbeddedData();
#endif

#ifdef __EMSCRIPTEN__
    GDALRegister_MEM();
    GDALRegister_PNG();
    GDALRegister_GTiff();
    GDALRegister_AAIGrid();
#else
    GDALAllRegister();
#endif

    set_log_handler();
}

void unregisterGdal()
{
#ifdef EMBED_GDAL_DATA
    unregisterEmbeddedData();
    destroy_embedded_data();
#endif

    GDALDestroy();
}

void registerEmbeddedData()
{
#ifdef EMBED_GDAL_DATA
    register_embedded_data_file_finder();
#endif
}

void unregisterEmbeddedData()
{
#ifdef EMBED_GDAL_DATA
    unregister_embedded_data_file_finder();
#endif
}

std::vector<const char*> create_string_array(const std::vector<std::string>& driverOptions)
{
    std::vector<const char*> options(driverOptions.size());
    std::transform(driverOptions.begin(), driverOptions.end(), options.begin(), [](auto& str) {
        return str.c_str();
    });
    options.push_back(nullptr);
    return options;
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
    return RasterDataSet(checkPointer(_driver.Create(filename.u8string().c_str(), cols, rows, numBands, resolveType(dataType), nullptr), "Failed to create data set"));
}

RasterDataSet RasterDriver::create_dataset(int32_t rows, int32_t cols, int32_t numBands, const std::type_info& dataType)
{
    return RasterDataSet(checkPointer(_driver.Create("", cols, rows, numBands, resolveType(dataType), nullptr), "Failed to create data set"));
}

RasterDataSet RasterDriver::create_dataset_copy(const RasterDataSet& reference, const fs::path& filename, const std::vector<std::string>& driverOptions)
{
    auto options = create_string_array(driverOptions);
    return RasterDataSet(checkPointer(_driver.CreateCopy(
                                          filename.u8string().c_str(),
                                          reference.get(),
                                          FALSE,
                                          options.size() == 1 ? nullptr : const_cast<char**>(options.data()),
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

VectorDataSet VectorDriver::create_dataset(const fs::path& filename)

{
    return VectorDataSet(checkPointer(_driver.Create(filename.u8string().c_str(), 0, 0, 0, GDT_Unknown, nullptr), "Failed to create vector data set"));
}

VectorDataSet VectorDriver::create_dataset_copy(const VectorDataSet& reference, const fs::path& filename, const std::vector<std::string>& driverOptions)
{
    auto options = create_string_array(driverOptions);
    return VectorDataSet(checkPointer(_driver.CreateCopy(
                                          filename.u8string().c_str(),
                                          reference.get(),
                                          FALSE,
                                          options.size() == 1 ? nullptr : const_cast<char**>(options.data()),
                                          nullptr,
                                          nullptr),
        "Failed to create data set copy"));
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
    std::string path = filePath.generic_u8string();

    auto options = create_string_array(driverOpts);
    return reinterpret_cast<GDALDataset*>(GDALOpenEx(
        path.c_str(),
        openFlags,
        drivers,
        options.size() == 1 ? nullptr : options.data(),
        nullptr));
}

RasterDataSet RasterDataSet::create(const fs::path& filePath, const std::vector<std::string>& driverOpts)
{
    auto* dataSet = create_data_set(filePath, GDAL_OF_READONLY | GDAL_OF_RASTER, nullptr, driverOpts);
    if (!dataSet) {
        throw RuntimeError("Failed to open raster file '{}'", filePath);
    }

    return RasterDataSet(dataSet);
}

RasterDataSet RasterDataSet::create(const fs::path& filePath, RasterType type, const std::vector<std::string>& driverOpts)
{
    if (type == RasterType::Unknown) {
        type = guess_rastertype_from_filename(filePath);
        if (type == RasterType::Unknown) {
            throw RuntimeError("Failed to determine raster type for file ('{}')", filePath);
        }
    }

    return RasterDataSet(checkPointer(create_data_set(filePath,
                                          GDAL_OF_READONLY | GDAL_OF_RASTER,
                                          nullptr,
                                          driverOpts),
        "Failed to open raster file"));
}

RasterDataSet::RasterDataSet(GDALDataset* ptr) noexcept
: _ptr(ptr)
{
}

RasterDataSet::RasterDataSet(RasterDataSet&& rhs)
: _ptr(rhs._ptr)
{
    rhs._ptr = nullptr;
}

RasterDataSet::~RasterDataSet() noexcept
{
    GDALClose(reinterpret_cast<GDALDatasetH>(_ptr));
}

RasterDataSet& RasterDataSet::operator=(RasterDataSet&& rhs)
{
    if (_ptr) {
        GDALClose(reinterpret_cast<GDALDatasetH>(_ptr));
    }

    _ptr     = rhs._ptr;
    rhs._ptr = nullptr;
    return *this;
}

bool RasterDataSet::is_valid() const
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

std::array<double, 6> RasterDataSet::geotransform() const
{
    assert(_ptr);
    std::array<double, 6> trans;
    checkError(_ptr->GetGeoTransform(trans.data()), "Failed to get extent metadata");
    return trans;
}

void RasterDataSet::set_geotransform(const std::array<double, 6>& trans)
{
    assert(_ptr);
    checkError(_ptr->SetGeoTransform(const_cast<double*>(trans.data())), "Failed to set geo transform");
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

    auto* band = _ptr->GetRasterBand(bandNr);
    if (band == nullptr) {
        throw RuntimeError("Invalid dataset band number: {}", bandNr);
    }

    if (value) {
        checkError(band->SetNoDataValue(*value), "Failed to set nodata value");
    } else {
        checkError(band->DeleteNoDataValue(), "Failed to delete nodata value");
    }
}

void RasterDataSet::set_colortable(int bandNr, const GDALColorTable* ct)
{
    assert(_ptr);
    assert(bandNr > 0);
    auto* band = checkPointer(_ptr->GetRasterBand(bandNr), "Failed to get raster band");
    checkError(band->SetColorTable(const_cast<GDALColorTable*>(ct)), "Failed to set color table");
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
        checkError(_ptr->SetProjection(proj.c_str()), "Failed to set projection");
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

std::unordered_map<std::string, std::string> RasterDataSet::metadata(const std::string& domain)
{
    std::unordered_map<std::string, std::string> result;

    char** data = _ptr->GetMetadata(domain.c_str());
    int index   = 0;
    while (data[index] != nullptr) {
        auto keyValue = str::split_view(data[index++], '=');
        if (keyValue.size() == 2) {
            result.emplace(keyValue[0], keyValue[1]);
        }
    }

    return result;
}

void RasterDataSet::set_metadata(const std::string& name, const std::string& value, const std::string& domain)
{
    checkError(_ptr->SetMetadataItem(name.c_str(), value.c_str(), domain.c_str()), "Failed to set metadata");
}

std::vector<std::string> RasterDataSet::metadata_domains() const
{
    std::vector<std::string> result;

    char** data = checkPointer(_ptr->GetMetadataDomainList(), "Failed to obtain metadata domains");
    int index   = 0;
    while (data[index] != nullptr) {
        result.push_back(data[index++]);
    }

    CSLDestroy(data);

    return result;
}

RasterBand RasterDataSet::rasterband(int bandNr) const
{
    assert(bandNr > 0);
    return RasterBand(checkPointer(_ptr->GetRasterBand(bandNr), "Invalid band index"));
}

GDALDataType RasterDataSet::band_datatype(int bandNr) const
{
    assert(_ptr);
    assert(bandNr > 0);
    return checkPointer(_ptr->GetRasterBand(bandNr), "Invalid band index")->GetRasterDataType();
}

void RasterDataSet::read_rasterdata(int band, int xOff, int yOff, int xSize, int ySize, const std::type_info& type, void* pData, int bufXSize, int bufYSize, int pixelSize, int lineSize) const
{
    auto* bandPtr = _ptr->GetRasterBand(band);
    checkError(bandPtr->RasterIO(GF_Read, xOff, yOff, xSize, ySize, pData, bufXSize, bufYSize, resolveType(type), pixelSize, lineSize), "Failed to read raster data");
}

void RasterDataSet::write_rasterdata(int band, int xOff, int yOff, int xSize, int ySize, const std::type_info& type, const void* pData, int bufXSize, int bufYSize) const
{
    auto* bandPtr = _ptr->GetRasterBand(band);
    checkError(bandPtr->RasterIO(GF_Write, xOff, yOff, xSize, ySize, const_cast<void*>(pData), bufXSize, bufYSize, resolveType(type), 0, 0), "Failed to write raster data");
}

void RasterDataSet::write_geometadata(const GeoMetadata& meta)
{
    if (raster_count() > 0) {
        set_nodata_value(1, meta.nodata);
    }

    set_geotransform(std::array<double, 6>{{meta.xll, meta.cellSize, 0.0, meta.yll + (meta.cellSize * meta.rows), 0.0, -meta.cellSize}});
    set_projection(meta.projection);
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

VectorDataSet VectorDataSet::create(const fs::path& filePath, const std::vector<std::string>& driverOptions)
{
    auto* dsPtr = create_data_set(filePath, GDAL_OF_READONLY | GDAL_OF_VECTOR, nullptr, driverOptions);
    if (!dsPtr) {
        throw RuntimeError("Failed to open vector file '{}'", filePath);
    }

    return VectorDataSet(dsPtr);
}

VectorDataSet VectorDataSet::create(const fs::path& filePath, VectorType type, const std::vector<std::string>& driverOptions)
{
    if (type == VectorType::Unknown) {
        type = guess_vectortype_from_filename(filePath);
        if (type == VectorType::Unknown) {
            throw RuntimeError("Failed to determine vector type for file ('{}')", filePath);
        }
    }

    std::array<const char*, 2> drivers{{s_vectorDriverLookup.at(type), nullptr}};

    auto* dsPtr = create_data_set(filePath, GDAL_OF_READONLY | GDAL_OF_VECTOR, drivers.data(), driverOptions);
    if (!dsPtr) {
        throw RuntimeError("Failed to open vector file '{}'", filePath);
    }

    return VectorDataSet(dsPtr);
}

VectorDataSet::VectorDataSet(GDALDataset* ptr) noexcept
: _ptr(ptr)
{
}

VectorDataSet::VectorDataSet(VectorDataSet&& rhs)
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
        checkError(_ptr->SetProjection(proj.c_str()), "Failed to set projection");
    }
}

void VectorDataSet::set_metadata(const std::string& name, const std::string& value, const std::string& domain)
{
    checkError(_ptr->SetMetadataItem(name.c_str(), value.c_str(), domain.c_str()), "Failed to set metadata");
}

Layer VectorDataSet::layer(int index)
{
    assert(_ptr);
    return Layer(checkPointer(_ptr->GetLayer(index), "Invalid layer index"));
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
        return wkbCurve;
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
    auto options = create_string_array(driverOptions);
    return Layer(checkPointer(_ptr->CreateLayer(name.c_str(), nullptr, to_gdal_type(type), const_cast<char**>(options.data())), "Layer creation failed"));
}

GDALDataset* VectorDataSet::get() const
{
    return _ptr;
}

VectorDriver VectorDataSet::driver()
{
    return VectorDriver(*_ptr->GetDriver());
}

void throwLastError(const char* msg)
{
    auto* errorMsg = CPLGetLastErrorMsg();
    if (errorMsg != nullptr && strlen(errorMsg) > 0) {
        throw RuntimeError("{}: {}", msg, errorMsg);
    } else {
        throw RuntimeError(msg);
    }
}

void checkError(CPLErr err, const char* msg)
{
    if (err != CE_None) {
        throwLastError(msg);
    }
}

void checkError(OGRErr err, const char* msg)
{
    if (err != OGRERR_NONE) {
        throwLastError(msg);
    }
}

void checkError(CPLErr err, const std::string& msg)
{
    checkError(err, msg.c_str());
}

void checkError(OGRErr err, const std::string& msg)
{
    checkError(err, msg.c_str());
}

RasterType guess_rastertype_from_filename(const fs::path& filePath)
{
    auto ext = getExtenstion(filePath);
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
    }

    return RasterType::Unknown;
}

VectorType guess_vectortype_from_filename(const fs::path& filePath)
{
    auto ext = getExtenstion(filePath);
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
        meta.cellSize = geoTrans[1];
    }

    // transform the lower left coordinate (0.0, meta.rows)
    meta.xll = geoTrans[0] + geoTrans[1] * 0.0 + geoTrans[2] * meta.rows;
    meta.yll = geoTrans[3] + geoTrans[4] * 0.0 + geoTrans[5] * meta.rows;
}

MemoryFile::MemoryFile(std::string path, gsl::span<const uint8_t> dataBuffer)
: _path(std::move(path))
, _ptr(VSIFileFromMemBuffer(_path.c_str(),
      const_cast<GByte*>(reinterpret_cast<const GByte*>(dataBuffer.data())),
      dataBuffer.size(), FALSE /*no ownership*/))
{
}

const std::string& MemoryFile::path() const
{
    return _path;
}

MemoryFile::~MemoryFile()
{
    VSIFCloseL(_ptr);
}

}
