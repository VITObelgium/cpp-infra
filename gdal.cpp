#include "infra/gdal.h"
#include "infra/exception.h"
#include "infra/filesystem.h"
#include "infra/gdallog.h"
#include "infra/log.h"

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
    }};

static const std::unordered_map<std::string, RasterType> s_rasterDriverDescLookup{{
    {"MEM", RasterType::Memory},
    {"AAIGrid", RasterType::ArcAscii},
    {"GTiff", RasterType::GeoTiff},
    {"GIF", RasterType::Gif},
    {"PNG", RasterType::Png},
    {"PCRaster", RasterType::PcRaster},
}};

static const std::unordered_map<VectorType, const char*> s_vectorDriverLookup{{
    {VectorType::Memory, "Memory"},
    {VectorType::Csv, "CSV"},
    {VectorType::Tab, "CSV"},
    {VectorType::ShapeFile, "ESRI Shapefile"},
    {VectorType::Xlsx, "XLSX"},
}};

static const std::unordered_map<std::string, VectorType> s_vectorDriverDescLookup{{
    {"Memory", VectorType::Memory},
    {"CSV", VectorType::Csv},
    {"CSV", VectorType::Tab},
    {"ESRI Shapefile", VectorType::ShapeFile},
    {"XLSX", VectorType::Xlsx},
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

CoordinateTransformer::CoordinateTransformer(int32_t sourceEpsg, int32_t destEpsg)
{
    _sourceSRS.importFromEPSG(sourceEpsg);
    _targetSRS.importFromEPSG(destEpsg);

    auto* transformation = OGRCreateCoordinateTransformation(&_sourceSRS, &_targetSRS);
    if (!transformation) {
        throw RuntimeError("Failed to create transformation from EPSG:{} to EPSG:{}", sourceEpsg, destEpsg);
    }

    _transformer.reset(transformation);
}

Point<double> CoordinateTransformer::transform(const Point<double>& point) const
{
    Point<double> result = point;
    if (!_transformer->Transform(1, &result.x, &result.y)) {
        throw RuntimeError("Failed to transform point ({}, {})", point.x, point.y);
    }

    return result;
}

void CoordinateTransformer::transformInPlace(Point<double>& point) const
{
    if (!_transformer->Transform(1, &point.x, &point.y)) {
        throw RuntimeError("Failed to perform transformation");
    }
}

OGRCoordinateTransformation* CoordinateTransformer::get()
{
    return _transformer.get();
}

Point<double> convertPointProjected(int32_t sourceEpsg, int32_t destEpsg, Point<double> point)
{
    CoordinateTransformer transformer(sourceEpsg, destEpsg);
    return transformer.transform(point);
}

Point<double> projectedToGeoGraphic(int32_t epsg, Point<double> point)
{
    OGRSpatialReference utm, *poLatLong;
    utm.importFromEPSG(epsg);
    utm.SetProjCS("UTM 17 / WGS84");
    utm.SetWellKnownGeogCS("WGS84");
    utm.SetUTM(17);

    poLatLong  = utm.CloneGeogCS();
    auto trans = checkPointer(OGRCreateCoordinateTransformation(&utm, poLatLong),
        "Failed to create transformation");

    if (!trans->Transform(1, &point.x, &point.y)) {
        throw RuntimeError("Failed to perform transformation");
    }

    return point;
}

std::string projectionToFriendlyName(const std::string& projection)
{
    OGRSpatialReference spatialRef;
    checkError(spatialRef.importFromWkt(projection.c_str()), "Failed to import projection WKT");
    CplPointer<char> friendlyWkt;
    checkError(spatialRef.exportToPrettyWkt(friendlyWkt.ptrAddress(), TRUE), "Failed to export projection to pretty WKT");

    return std::string(friendlyWkt);
}

std::string projectionFromEpsg(int32_t epsg)
{
    OGRSpatialReference spatialRef;
    checkError(spatialRef.importFromEPSG(epsg), fmt::format("Failed to create projection from epsg:{}", epsg));

    CplPointer<char> friendlyWkt;
    spatialRef.exportToWkt(friendlyWkt.ptrAddress());
    return std::string(friendlyWkt);
}

int32_t projectionToGeoEpsg(const std::string& projection)
{
    OGRSpatialReference spatialRef;
    checkError(spatialRef.importFromWkt(projection.c_str()), "Failed to import projection WKT");

    auto epsg = spatialRef.GetEPSGGeogCS();
    if (epsg < 0) {
        throw RuntimeError("Failed to determine epsg from projection");
    }

    return epsg;
}

int32_t projectionToEpsg(const std::string& projection)
{
    OGRSpatialReference spatialRef;
    checkError(spatialRef.importFromWkt(projection.c_str()), "Failed to import projection WKT");

    std::string epsgCode;
    return std::atoi(checkPointer(spatialRef.GetAuthorityCode("PROJCS"), "Failed to get projection authority code"));
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
    createEmbeddedData();
#endif
}

EmbeddedDataRegistration::~EmbeddedDataRegistration()
{
#ifdef EMBED_GDAL_DATA
    destroyEmbeddedData();
#endif
}

void registerGdal()
{
#ifdef EMBED_GDAL_DATA
    createEmbeddedData();
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

    setLogHandler();
}

void unregisterGdal()
{
#ifdef EMBED_GDAL_DATA
    unregisterEmbeddedData();
    destroyEmbeddedData();
#endif

    GDALDestroy();
}

void registerEmbeddedData()
{
#ifdef EMBED_GDAL_DATA
    registerEmbeddedDataFileFinder();
#endif
}

void unregisterEmbeddedData()
{
#ifdef EMBED_GDAL_DATA
    unregisterEmbeddedDataFileFinder();
#endif
}

std::vector<const char*> createOptionsArray(const std::vector<std::string>& driverOptions)
{
    std::vector<const char*> options(driverOptions.size());
    std::transform(driverOptions.begin(), driverOptions.end(), options.begin(), [](auto& str) {
        return str.c_str();
    });
    options.push_back(nullptr);
    return options;
}

bool RasterDriver::isSupported(RasterType type)
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
    auto rasterType = guessRasterTypeFromFileName(filename);
    if (rasterType != RasterType::Unknown) {
        return create(rasterType);
    }

    throw RuntimeError("Failed to determine raster type from filename: {}", filename.string());
}

RasterDriver::RasterDriver(GDALDriver& driver)
: _driver(driver)
{
}

RasterDataSet RasterDriver::createDataSet(int32_t rows, int32_t cols, int32_t numBands, const fs::path& filename, const std::type_info& dataType)
{
    return RasterDataSet(checkPointer(_driver.Create(filename.string().c_str(), cols, rows, numBands, resolveType(dataType), nullptr), "Failed to create data set"));
}

RasterDataSet RasterDriver::createDataSet(int32_t rows, int32_t cols, int32_t numBands, const std::type_info& dataType)
{
    return RasterDataSet(checkPointer(_driver.Create("", cols, rows, numBands, resolveType(dataType), nullptr), "Failed to create data set"));
}

RasterDataSet RasterDriver::createDataSetCopy(const RasterDataSet& reference, const fs::path& filename, const std::vector<std::string>& driverOptions)
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

RasterType RasterDriver::type() const
{
    try {
        return s_rasterDriverDescLookup.at(_driver.GetDescription());
    } catch (const std::out_of_range&) {
        throw RuntimeError("Failed to determine raster type for driver: {}", _driver.GetDescription());
    }
}

bool VectorDriver::isSupported(VectorType type)
{
    if (type == VectorType::Unknown) {
        throw InvalidArgument("Invalid vector type specified");
    }

    return nullptr != GetGDALDriverManager()->GetDriverByName(s_vectorDriverLookup.at(type));
}

VectorDriver VectorDriver::create(const fs::path& filename)
{
    auto vectorType = guessVectorTypeFromFileName(filename.string());
    if (vectorType != VectorType::Unknown) {
        return create(vectorType);
    }

    throw RuntimeError("Failed to determine vector type from filename: {}", filename.string());
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

VectorDataSet VectorDriver::createDataSet()

{
    return createDataSet("");
}

VectorDataSet VectorDriver::createDataSet(const fs::path& filename)

{
    return VectorDataSet(checkPointer(_driver.Create(filename.string().c_str(), 0, 0, 0, GDT_Unknown, nullptr), "Failed to create vector data set"));
}

VectorDataSet VectorDriver::createDataSetCopy(const VectorDataSet& reference, const fs::path& filename, const std::vector<std::string>& driverOptions)
{
    auto options = createOptionsArray(driverOptions);
    return VectorDataSet(checkPointer(_driver.CreateCopy(
                                          filename.string().c_str(),
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

static GDALDataset* createDataSet(const fs::path& filePath,
    unsigned int openFlags,
    const char* const* drivers,
    const std::vector<std::string>& driverOpts)
{
    // use generic_string otherwise the path contains backslashes on windows
    // In memory file paths like /vsimem/file.asc in memory will then be \\vsimem\\file.asc
    // which is not recognized by gdal
    std::string path = filePath.generic_string();

    auto options = createOptionsArray(driverOpts);
    return reinterpret_cast<GDALDataset*>(GDALOpenEx(
        path.c_str(),
        openFlags,
        drivers,
        options.size() == 1 ? nullptr : options.data(),
        nullptr));
}

RasterDataSet RasterDataSet::create(const fs::path& filePath, const std::vector<std::string>& driverOpts)
{
    auto* dataSet = createDataSet(filePath, GDAL_OF_READONLY | GDAL_OF_RASTER, nullptr, driverOpts);
    if (!dataSet) {
        throw RuntimeError("Failed to open raster file '{}'", filePath.string());
    }

    return RasterDataSet(dataSet);
}

RasterDataSet RasterDataSet::create(const fs::path& filePath, RasterType type, const std::vector<std::string>& driverOpts)
{
    if (type == RasterType::Unknown) {
        type = guessRasterTypeFromFileName(filePath);
        if (type == RasterType::Unknown) {
            throw RuntimeError("Failed to determine raster type for file ('{}')", filePath.string());
        }
    }

    return RasterDataSet(checkPointer(createDataSet(filePath,
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

int32_t RasterDataSet::rasterCount() const
{
    assert(_ptr);
    return _ptr->GetRasterCount();
}

int32_t RasterDataSet::xSize() const
{
    assert(_ptr);
    return _ptr->GetRasterXSize();
}

int32_t RasterDataSet::ySize() const
{
    assert(_ptr);
    return _ptr->GetRasterYSize();
}

std::array<double, 6> RasterDataSet::geoTransform() const
{
    assert(_ptr);
    std::array<double, 6> trans;
    checkError(_ptr->GetGeoTransform(trans.data()), "Failed to get extent metadata");
    return trans;
}

void RasterDataSet::setGeoTransform(const std::array<double, 6>& trans)
{
    assert(_ptr);
    checkError(_ptr->SetGeoTransform(const_cast<double*>(trans.data())), "Failed to set geo transform");
}

std::optional<double> RasterDataSet::noDataValue(int bandNr) const
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

void RasterDataSet::setNoDataValue(int bandNr, std::optional<double> value) const
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

void RasterDataSet::setColorTable(int bandNr, const GDALColorTable* ct)
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

void RasterDataSet::setProjection(const std::string& proj)
{
    assert(_ptr);
    if (!proj.empty()) {
        checkError(_ptr->SetProjection(proj.c_str()), "Failed to set projection");
    }
}

void RasterDataSet::setMetadata(const std::string& name, const std::string& value, const std::string& domain)
{
    checkError(_ptr->SetMetadataItem(name.c_str(), value.c_str(), domain.c_str()), "Failed to set metadata");
}

RasterBand RasterDataSet::rasterBand(int bandNr) const
{
    return RasterBand(checkPointer(_ptr->GetRasterBand(bandNr), "Invalid band index"));
}

GDALDataType RasterDataSet::getBandDataType(int bandNr) const
{
    assert(_ptr);
    assert(bandNr > 0);
    return checkPointer(_ptr->GetRasterBand(bandNr), "Invalid band index")->GetRasterDataType();
}

void RasterDataSet::readRasterData(int band, int xOff, int yOff, int xSize, int ySize, const std::type_info& type, void* pData, int bufXSize, int bufYSize, int pixelSize, int lineSize) const
{
    auto* bandPtr = _ptr->GetRasterBand(band);
    checkError(bandPtr->RasterIO(GF_Read, xOff, yOff, xSize, ySize, pData, bufXSize, bufYSize, resolveType(type), pixelSize, lineSize), "Failed to read raster data");
}

void RasterDataSet::writeRasterData(int band, int xOff, int yOff, int xSize, int ySize, const std::type_info& type, const void* pData, int bufXSize, int bufYSize) const
{
    auto* bandPtr = _ptr->GetRasterBand(band);
    checkError(bandPtr->RasterIO(GF_Write, xOff, yOff, xSize, ySize, const_cast<void*>(pData), bufXSize, bufYSize, resolveType(type), 0, 0), "Failed to write raster data");
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
    auto* dsPtr = createDataSet(filePath, GDAL_OF_READONLY | GDAL_OF_VECTOR, nullptr, driverOptions);
    if (!dsPtr) {
        throw RuntimeError("Failed to open vector file '{}'", filePath.string());
    }

    return VectorDataSet(dsPtr);
}

VectorDataSet VectorDataSet::create(const fs::path& filePath, VectorType type, const std::vector<std::string>& driverOptions)
{
    if (type == VectorType::Unknown) {
        type = guessVectorTypeFromFileName(filePath);
        if (type == VectorType::Unknown) {
            throw RuntimeError("Failed to determine vector type for file ('{}')", filePath.string());
        }
    }

    std::array<const char*, 2> drivers{{s_vectorDriverLookup.at(type), nullptr}};

    auto* dsPtr = createDataSet(filePath, GDAL_OF_READONLY | GDAL_OF_VECTOR, drivers.data(), driverOptions);
    if (!dsPtr) {
        throw RuntimeError("Failed to open vector file '{}'", filePath.string());
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

int32_t VectorDataSet::layerCount() const
{
    assert(_ptr);
    return _ptr->GetLayerCount();
}

std::string VectorDataSet::projection() const
{
    assert(_ptr);
    return _ptr->GetProjectionRef();
}

void VectorDataSet::setProjection(const std::string& proj)
{
    assert(_ptr);
    if (!proj.empty()) {
        checkError(_ptr->SetProjection(proj.c_str()), "Failed to set projection");
    }
}

void VectorDataSet::setMetadata(const std::string& name, const std::string& value, const std::string& domain)
{
    checkError(_ptr->SetMetadataItem(name.c_str(), value.c_str(), domain.c_str()), "Failed to set metadata");
}

Layer VectorDataSet::getLayer(int index)
{
    assert(_ptr);
    return Layer(checkPointer(_ptr->GetLayer(index), "Invalid layer index"));
}

Layer VectorDataSet::createLayer(const std::string& name, const std::vector<std::string>& driverOptions)
{
    return createLayer(name, Geometry::Type::Unknown, driverOptions);
}

static OGRwkbGeometryType toGdalType(Geometry::Type type)
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

Layer VectorDataSet::createLayer(const std::string& name, Geometry::Type type, const std::vector<std::string>& driverOptions)
{
    assert(_ptr);
    auto options = createOptionsArray(driverOptions);
    return Layer(checkPointer(_ptr->CreateLayer(name.c_str(), nullptr, toGdalType(type), const_cast<char**>(options.data())), "Layer creation failed"));
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

RasterType guessRasterTypeFromFileName(const fs::path& filePath)
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
    }

    return RasterType::Unknown;
}

VectorType guessVectorTypeFromFileName(const fs::path& filePath)
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
    }

    return VectorType::Unknown;
}

static void fillMetadataFromGeoTransform(inf::GeoMetadata& meta, const std::array<double, 6>& geoTrans)
{
    if (geoTrans[2] == 0.0 && geoTrans[4] == 0.0) {
        meta.cellSize = geoTrans[1];
    }

    // transform the lower left coordinate (0.0, meta.rows)
    meta.xll = geoTrans[0] + geoTrans[1] * 0.0 + geoTrans[2] * meta.rows;
    meta.yll = geoTrans[3] + geoTrans[4] * 0.0 + geoTrans[5] * meta.rows;
}

inf::GeoMetadata readMetadataFromDataset(const gdal::RasterDataSet& dataSet)
{
    inf::GeoMetadata meta;

    if (dataSet.rasterCount() != 1) {
        throw RuntimeError("Only rasters with a single band are currently supported");
    }

    meta.cols       = dataSet.xSize();
    meta.rows       = dataSet.ySize();
    meta.nodata     = dataSet.noDataValue(1);
    meta.projection = dataSet.projection();
    fillMetadataFromGeoTransform(meta, dataSet.geoTransform());

    return meta;
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
