#include "infra/gdal.h"
#include "infra/exception.h"
#include "infra/log.h"

#ifdef EMBED_GDAL_DATA
#include "embedgdaldata.h"
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#ifdef HAVE_EXP_FILESYSTEM_H
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include <ogr_spatialref.h>

namespace infra::gdal {

using namespace std::string_literals;

namespace {

static const std::unordered_map<MapType, const char*> s_driverLookup{{{MapType::Memory, "MEM"},
    {MapType::ArcAscii, "AAIGrid"},
    {MapType::GeoTiff, "GTiff"},
    {MapType::Gif, "GIF"},
    {MapType::Png, "PNG"}}};

static const std::unordered_map<std::string, MapType> s_driverDescLookup{{{"MEM", MapType::Memory},
    {"AAIGrid", MapType::ArcAscii},
    {"GTiff", MapType::GeoTiff},
    {"GIF", MapType::Gif},
    {"PNG", MapType::Png}}};

static const std::unordered_map<VectorType, const char*> s_shapeDriverLookup{{
    {VectorType::Csv, "CSV"},
    {VectorType::Tab, "CSV"},
    {VectorType::ShapeFile, "ESRI Shapefile"},
}};

static std::string getExtenstion(const std::string& path)
{
#ifdef HAVE_EXP_FILESYSTEM_H
    return fs::path(path.begin(), path.end()).extension().string();
#else
    std::string extension;
    std::string::size_type pos = path.find_last_of('.');
    if (pos != std::string::npos && pos != path.size()) {
        extension = std::string(path.substr(pos, path.size()));
    }

    return extension;
#endif
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

Point<double> convertPointProjected(int32_t sourceEpsg, int32_t destEpsg, Point<double> point)
{
    OGRSpatialReference sourceSRS, targetSRS;
    sourceSRS.importFromEPSG(sourceEpsg);
    targetSRS.importFromEPSG(destEpsg);

    auto trans = checkPointer(OGRCreateCoordinateTransformation(&sourceSRS, &targetSRS),
        "Failed to create transformation");

    if (!trans->Transform(1, &point.x, &point.y)) {
        throw RuntimeError("Failed to perform transformation");
    }

    return Point<double>(point.x, point.y);
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
    auto projectionPtr = projection.c_str();
    checkError(spatialRef.importFromWkt(const_cast<char**>(&projectionPtr)), "Failed to import projection WKT");
    char* friendlyWkt = nullptr;
    checkError(spatialRef.exportToPrettyWkt(&friendlyWkt, TRUE), "Failed to export projection to pretty WKT");

    std::string result(friendlyWkt);
    CPLFree(friendlyWkt);
    return result;
}

std::string projectionFromEpsg(int32_t epsg)
{
    OGRSpatialReference spatialRef;
    checkError(spatialRef.importFromEPSG(epsg), fmt::format("Failed to create projection from epsg:{}", epsg));

    char* friendlyWkt = nullptr;
    spatialRef.exportToWkt(&friendlyWkt);
    std::string result(friendlyWkt);
    CPLFree(friendlyWkt);
    return result;
}

int32_t projectionToGeoEpsg(const std::string& projection)
{
    OGRSpatialReference spatialRef;
    auto projectionPtr = projection.c_str();
    checkError(spatialRef.importFromWkt(const_cast<char**>(&projectionPtr)), "Failed to import projection WKT");

    auto epsg = spatialRef.GetEPSGGeogCS();
    if (epsg < 0) {
        throw RuntimeError("Failed to determine epsg from projection");
    }

    return epsg;
}

int32_t projectionToEpsg(const std::string& projection)
{
    OGRSpatialReference spatialRef;
    auto projectionPtr = projection.c_str();
    checkError(spatialRef.importFromWkt(const_cast<char**>(&projectionPtr)), "Failed to import projection WKT");

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
    createEmbeddedData();
}

EmbeddedDataRegistration::~EmbeddedDataRegistration()
{
    destroyEmbeddedData();
}

void registerGdal()
{
#ifdef EMBED_GDAL_DATA
    createEmbeddedData();
#endif

#ifdef __EMSCRIPTEN__
    GDALRegister_MEM();
    GDALRegister_PNG();
    GDALRegister_GTiff();
    GDALRegister_AAIGrid();
#else
    GDALAllRegister();
#endif
}

void unregisterGdal()
{
#ifdef EMBED_GDAL_DATA
    destroyEmbeddedData();
#endif

    //CPLCleanupTLS();
    GDALDestroy();
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

Driver Driver::create(MapType mt)
{
    if (mt == MapType::Unknown) {
        throw InvalidArgument("Invalid map type specified");
    }

    auto driverName = s_driverLookup.at(mt);
    auto* driverPtr = GetGDALDriverManager()->GetDriverByName(driverName);
    if (driverPtr == nullptr) {
        throw RuntimeError("Failed to get driver: {}", driverName);
    }

    //    auto meta = driverPtr->GetMetadata();
    //    if (!CSLFetchBoolean(meta, GDAL_DCAP_CREATECOPY, FALSE))
    //    {
    //        throw RuntimeError("{} does not support map creation", driverName);
    //    }

    return Driver(driverPtr);
}

Driver Driver::create(VectorType mt)
{
    if (mt == VectorType::Unknown) {
        throw InvalidArgument("Invalid vector type specified");
    }

    auto driverName = s_shapeDriverLookup.at(mt);
    auto* driverPtr = GetGDALDriverManager()->GetDriverByName(driverName);
    if (driverPtr == nullptr) {
        throw RuntimeError("Failed to get driver: {}", driverName);
    }

    return Driver(driverPtr);
}

Driver Driver::create(const std::string& filename)
{
    auto mapType = guessMapTypeFromFileName(filename);
    if (mapType != MapType::Unknown) {
        return create(mapType);
    }

    auto shapeType = guessVectorTypeFromFileName(filename);
    if (shapeType != VectorType::Unknown) {
        return create(shapeType);
    }

    throw RuntimeError("Failed to determine type from filename: {}", filename);
}

MapType Driver::mapType() const
{
    try {
        return s_driverDescLookup.at(_driver->GetDescription());
    } catch (const std::out_of_range&) {
        throw RuntimeError("Failed to determine map type for driver: {}", _driver->GetDescription());
    }
}

DataSet DataSet::create(const std::string& filePath)
{
    return DataSet(filePath);
}

DataSet DataSet::create(const std::string& filePath, VectorType type, const std::vector<std::string>& driverOptions)
{
    std::array<const char*, 2> drivers{{s_shapeDriverLookup.at(type), nullptr}};

    auto options = createOptionsArray(driverOptions);
    return DataSet(checkPointer(reinterpret_cast<GDALDataset*>(GDALOpenEx(
                                    filePath.c_str(),
                                    GDAL_OF_READONLY | GDAL_OF_VECTOR,
                                    drivers.data(),
                                    options.size() == 1 ? nullptr : options.data(),
                                    nullptr)),
        "Failed to open vector file"));
}

DataSet::DataSet(GDALDataset* ptr) noexcept
: _ptr(ptr)
{
}

DataSet::DataSet(const std::string& filename)
: _ptr(checkPointer(reinterpret_cast<GDALDataset*>(GDALOpen(filename.c_str(), GA_ReadOnly)), "Failed to open file"))
{
}

DataSet::DataSet(DataSet&& rhs)
: _ptr(rhs._ptr)
{
    rhs._ptr = nullptr;
}

DataSet::~DataSet() noexcept
{
    GDALClose(reinterpret_cast<GDALDatasetH>(_ptr));
}

DataSet& DataSet::operator=(DataSet&& rhs)
{
    _ptr     = rhs._ptr;
    rhs._ptr = nullptr;
    return *this;
}

int32_t DataSet::rasterCount() const
{
    assert(_ptr);
    return _ptr->GetRasterCount();
}

int32_t DataSet::layerCount() const
{
    assert(_ptr);
    return _ptr->GetLayerCount();
}

int32_t DataSet::rasterXSize() const
{
    assert(_ptr);
    return _ptr->GetRasterXSize();
}

int32_t DataSet::rasterYSize() const
{
    assert(_ptr);
    return _ptr->GetRasterYSize();
}

std::array<double, 6> DataSet::geoTransform() const
{
    assert(_ptr);
    std::array<double, 6> trans;
    checkError(_ptr->GetGeoTransform(trans.data()), "Failed to get extent metadata");
    return trans;
}

void DataSet::setGeoTransform(const std::array<double, 6>& trans)
{
    assert(_ptr);
    checkError(_ptr->SetGeoTransform(const_cast<double*>(trans.data())), "Failed to set geo transform");
}

//std::optional<double> DataSet::noDataValue(int bandNr) const
//{
//    assert(bandNr > 0);
//
//    auto* band = _ptr->GetRasterBand(bandNr);
//    if (band == nullptr) {
//        throw RuntimeError("Invalid dataset band number: {}", bandNr);
//    }
//
//    int success = 0;
//    auto value  = band->GetNoDataValue(&success);
//    if (success) {
//        return std::make_optional(value);
//    }
//
//    return std::optional<double>();
//}
//
//void DataSet::setNoDataValue(int bandNr, std::optional<double> value) const
//{
//    assert(bandNr > 0);
//
//    auto* band = _ptr->GetRasterBand(bandNr);
//    if (band == nullptr) {
//        throw RuntimeError("Invalid dataset band number: {}", bandNr);
//    }
//
//    if (value) {
//        checkError(band->SetNoDataValue(*value), "Failed to set nodata value");
//    } else {
//        checkError(band->DeleteNoDataValue(), "Failed to delete nodata value");
//    }
//}

void DataSet::setColorTable(int bandNr, const GDALColorTable* ct)
{
    assert(_ptr);
    assert(bandNr > 0);
    auto* band = checkPointer(_ptr->GetRasterBand(bandNr), "Failed to get raster band");
    checkError(band->SetColorTable(const_cast<GDALColorTable*>(ct)), "Failed to set color table");
}

std::string DataSet::projection() const
{
    assert(_ptr);
    return _ptr->GetProjectionRef();
}

void DataSet::setProjection(const std::string& proj)
{
    assert(_ptr);
    if (!proj.empty()) {
        checkError(_ptr->SetProjection(proj.c_str()), "Failed to set projection");
    }
}

OGRLayer& DataSet::getLayer(int index)
{
    assert(_ptr);
    return *checkPointer(_ptr->GetLayer(index), "Invalid layer index");
}

GDALDataType DataSet::getBandDataType(int bandNr) const
{
    assert(_ptr);
    assert(bandNr > 0);
    return checkPointer(_ptr->GetRasterBand(bandNr), "Invalid band index")->GetRasterDataType();
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

MapType guessMapTypeFromFileName(const std::string& filePath)
{
    auto ext = getExtenstion(filePath);
    if (ext == ".asc") {
        return MapType::ArcAscii;
    } else if (ext == ".tiff" || ext == ".tif") {
        return MapType::GeoTiff;
    } else if (ext == ".gif") {
        return MapType::Gif;
    } else if (ext == ".png") {
        return MapType::Png;
    }

    return MapType::Unknown;
}

VectorType guessVectorTypeFromFileName(const std::string& filePath)
{
    auto ext = getExtenstion(filePath);
    if (ext == ".csv") {
        return VectorType::Csv;
    } else if (ext == ".tab") {
        return VectorType::Tab;
    } else if (ext == ".tab") {
        return VectorType::ShapeFile;
    }

    return VectorType::Unknown;
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
