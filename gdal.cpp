#include "infra/gdal.h"
#include "infra/exception.h"
#include "infra/filesystem.h"
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

namespace infra::gdal {

using namespace std::string_literals;

namespace {

static const std::unordered_map<RasterType, const char*> s_driverLookup{{
    {RasterType::Memory, "MEM"},
    {RasterType::ArcAscii, "AAIGrid"},
    {RasterType::GeoTiff, "GTiff"},
    {RasterType::Gif, "GIF"},
    {RasterType::Png, "PNG"},
}};

static const std::unordered_map<std::string, RasterType> s_driverDescLookup{{
    {"MEM", RasterType::Memory},
    {"AAIGrid", RasterType::ArcAscii},
    {"GTiff", RasterType::GeoTiff},
    {"GIF", RasterType::Gif},
    {"PNG", RasterType::Png},
}};

static const std::unordered_map<VectorType, const char*> s_shapeDriverLookup{{
    {VectorType::Memory, "Memory"},
    {VectorType::Csv, "CSV"},
    {VectorType::Tab, "CSV"},
    {VectorType::ShapeFile, "ESRI Shapefile"},
    {VectorType::Xlsx, "XLSX"},
}};

static const std::unordered_map<std::string, VectorType> s_shapeDriverDescLookup{{
    {"Memory", VectorType::Memory},
    {"CSV", VectorType::Csv},
    {"CSV", VectorType::Tab},
    {"ESRI Shapefile", VectorType::ShapeFile},
    {"XLSX", VectorType::Xlsx},
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

Driver Driver::create(RasterType mt)
{
    if (mt == RasterType::Unknown) {
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

    return Driver(*driverPtr);
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

    return Driver(*driverPtr);
}

Driver Driver::create(const std::string& filename)
{
    auto rasterType = guessRasterTypeFromFileName(filename);
    if (rasterType != RasterType::Unknown) {
        return create(rasterType);
    }

    auto shapeType = guessVectorTypeFromFileName(filename);
    if (shapeType != VectorType::Unknown) {
        return create(shapeType);
    }

    throw RuntimeError("Failed to determine type from filename: {}", filename);
}

Driver::Driver(GDALDriver& driver)
: _driver(driver)
{
}

RasterType Driver::rasterType() const
{
    try {
        return s_driverDescLookup.at(_driver.GetDescription());
    } catch (const std::out_of_range&) {
        throw RuntimeError("Failed to determine map type for driver: {}", _driver.GetDescription());
    }
}

VectorType Driver::vectorType() const
{
    try {
        return s_shapeDriverDescLookup.at(_driver.GetDescription());
    } catch (const std::out_of_range&) {
        throw RuntimeError("Failed to determine vector type for driver: {}", _driver.GetDescription());
    }
}

Line::Line(OGRSimpleCurve* curve)
: _curve(curve)
{
    assert(curve);
}

int Line::pointCount() const
{
    return _curve->getNumPoints();
}

Point<double> Line::pointAt(int index) const
{
    OGRPoint p;
    _curve->getPoint(index, &p);
    return Point<double>(p.getX(), p.getY());
}

Point<double> Line::startPoint()
{
    OGRPoint point;
    _curve->StartPoint(&point);
    return Point<double>(point.getX(), point.getY());
}

Point<double> Line::endPoint()
{
    OGRPoint point;
    _curve->EndPoint(&point);
    return Point<double>(point.getX(), point.getY());
}

OGRSimpleCurve* Line::get()
{
    return _curve;
}

LineIterator::LineIterator(Line line)
: _iter(line.get()->getPointIterator())
{
    next();
}

LineIterator::~LineIterator()
{
    OGRPointIterator::destroy(_iter);
}

void LineIterator::next()
{
    OGRPoint p;
    if (!_iter->getNextPoint(&p)) {
        OGRPointIterator::destroy(_iter);
        _iter = nullptr;
    } else {
        _point.x = p.getX();
        _point.y = p.getY();
    }
}

const Point<double>& LineIterator::operator*()
{
    return _point;
}

const Point<double>* LineIterator::operator->()
{
    return &_point;
}

LineIterator& LineIterator::operator++()
{
    next();
    return *this;
}

LineIterator& LineIterator::operator=(LineIterator&& other)
{
    if (this != &other) {
        if (_iter) {
            OGRPointIterator::destroy(_iter);
        }

        _iter       = std::move(other._iter);
        other._iter = nullptr;
    }

    return *this;
}

bool LineIterator::operator==(const LineIterator& other) const
{
    return _iter == other._iter;
}

bool LineIterator::operator!=(const LineIterator& other) const
{
    return !(*this == other);
}

MultiLine::MultiLine(OGRMultiLineString* multiLine)
: _multiLine(multiLine)
{
    assert(_multiLine);
}

OGRMultiLineString* MultiLine::get()
{
    return _multiLine;
}

int MultiLine::geometryCount() const
{
    return _multiLine->getNumGeometries();
}

Line MultiLine::geometry(int index) const
{
    return Line(reinterpret_cast<OGRLineString*>(_multiLine->getGeometryRef(index)));
}

LinearRing::LinearRing(OGRLinearRing* ring)
: Line(ring)
, _ring(ring)
{
}

OGRLinearRing* LinearRing::get()
{
    return _ring;
}

Polygon::Polygon(OGRPolygon* poly)
: _poly(poly)
{
}

LinearRing Polygon::exteriorRing()
{
    return LinearRing(_poly->getExteriorRing());
}

LinearRing Polygon::interiorRing(int index)
{
    return LinearRing(_poly->getInteriorRing(index));
}

OGRPolygon* Polygon::get()
{
    return _poly;
}

static OGRFieldType fieldTypeFromTypeInfo(const std::type_info& typeInfo)
{
    if (typeInfo == typeid(int32_t)) {
        return OFTInteger;
    } else if (typeInfo == typeid(int64_t)) {
        return OFTInteger64;
    } else if (typeInfo == typeid(float) || typeInfo == typeid(double)) {
        return OFTReal;
    } else if (typeInfo == typeid(std::string_view) || typeInfo == typeid(std::string)) {
        return OFTString;
    }

    throw InvalidArgument("Invalid field type provided");
}

FieldDefinition::FieldDefinition(const char* name, const std::type_info& typeInfo)
: _hasOwnerShip(true)
, _def(new OGRFieldDefn(name, fieldTypeFromTypeInfo(typeInfo)))
{
}

FieldDefinition::FieldDefinition(OGRFieldDefn* def)
: _hasOwnerShip(false)
, _def(def)
{
}

FieldDefinition::~FieldDefinition()
{
    if (_hasOwnerShip) {
        delete _def;
    }
}

std::string_view FieldDefinition::name() const
{
    return std::string_view(_def->GetNameRef());
}

const std::type_info& FieldDefinition::type() const
{
    switch (_def->GetType()) {
    case OFTInteger:
        return typeid(int32_t);
    case OFTReal:
        return typeid(double);
    case OFTInteger64:
        return typeid(int64_t);
    case OFTString:
        return typeid(std::string_view);
    case OFTDate:
        return typeid(date_point);
    case OFTIntegerList:
    case OFTRealList:
    case OFTStringList:
    case OFTWideString:
    case OFTBinary:
    case OFTTime:
    case OFTDateTime:
    case OFTInteger64List:
    default:
        throw std::runtime_error("Type not implemented");
    }
}

OGRFieldDefn* FieldDefinition::get() noexcept
{
    return _def;
}

FeatureDefinition::FeatureDefinition(OGRFeatureDefn* def)
: _hasOwnerShip(false)
, _def(def)
{
}

FeatureDefinition::~FeatureDefinition()
{
    if (_hasOwnerShip) {
        delete _def;
    }
}

std::string_view FeatureDefinition::name() const
{
    return std::string_view(_def->GetName());
}

int FeatureDefinition::fieldCount() const
{
    return _def->GetFieldCount();
}

int FeatureDefinition::fieldIndex(std::string_view name) const
{
    return _def->GetFieldIndex(name.data());
}

FieldDefinition FeatureDefinition::fieldDefinition(int index) const
{
    return FieldDefinition(checkPointer(_def->GetFieldDefn(index), "Failed to obtain field definition"));
}

OGRFeatureDefn* FeatureDefinition::get() noexcept
{
    return _def;
}

Feature::Feature(Layer& layer)
: _feature(OGRFeature::CreateFeature(layer.get()->GetLayerDefn()))
{
}

Feature::Feature(OGRFeature* feature)
: _feature(feature)
{
}

Feature::Feature(Feature&& other)
: _feature(other._feature)
{
    other._feature = nullptr;
}

Feature::~Feature()
{
    OGRFeature::DestroyFeature(_feature);
}

Feature& Feature::operator=(Feature&& other)
{
    OGRFeature::DestroyFeature(_feature);
    _feature       = other._feature;
    other._feature = nullptr;
    return *this;
}

OGRFeature* Feature::get()
{
    return _feature;
}

const OGRFeature* Feature::get() const
{
    return _feature;
}

Geometry Feature::geometry()
{
    auto* geometry = _feature->GetGeometryRef();
    if (!geometry) {
        return Geometry();
    }

    switch (wkbFlatten(geometry->getGeometryType())) {
    case wkbPoint: {
        auto* point = reinterpret_cast<OGRPoint*>(geometry);
        return Point<double>(point->getX(), point->getY());
    }
    case wkbLineString:
        return Line(reinterpret_cast<OGRLineString*>(geometry));
    case wkbPolygon:
        return Polygon(reinterpret_cast<OGRPolygon*>(geometry));
    case wkbMultiLineString:
        return MultiLine(reinterpret_cast<OGRMultiLineString*>(geometry));
    default:
        throw RuntimeError("Unsupported geometry type ({})", wkbFlatten(geometry->getGeometryType()));
    }
}

const Geometry Feature::geometry() const
{
    return const_cast<Feature*>(this)->geometry();
}

int Feature::fieldCount() const
{
    return _feature->GetFieldCount();
}

int Feature::fieldIndex(std::string_view name) const
{
    return _feature->GetFieldIndex(name.data());
}

FieldDefinition Feature::fieldDefinition(int index) const
{
    return FieldDefinition(_feature->GetFieldDefnRef(index));
}

Field Feature::getField(int index) const noexcept
{
    auto& type = fieldDefinition(index).type();
    if (type == typeid(double)) {
        return Field(getFieldAs<double>(index));
    } else if (type == typeid(int32_t)) {
        return Field(getFieldAs<int32_t>(index));
    } else if (type == typeid(int64_t)) {
        return Field(getFieldAs<int64_t>(index));
    } else if (type == typeid(std::string_view)) {
        return Field(getFieldAs<std::string_view>(index));
    }

    return Field();
}

template <typename T>
T Feature::getFieldAs(int index) const
{
    if constexpr (std::is_same_v<double, T>) {
        return _feature->GetFieldAsDouble(index);
    } else if constexpr (std::is_same_v<float, T>) {
        return static_cast<float>(_feature->GetFieldAsDouble(index));
    } else if constexpr (std::is_same_v<int32_t, T>) {
        return _feature->GetFieldAsInteger(index);
    } else if constexpr (std::is_same_v<int64_t, T>) {
        return _feature->GetFieldAsInteger64(index);
    } else if constexpr (std::is_same_v<std::string_view, T>) {
        return std::string_view(_feature->GetFieldAsString(index));
    } else {
        throw std::invalid_argument("Invalid field type");
    }
}

template <typename T>
T Feature::getFieldAs(std::string_view name) const
{
    if constexpr (std::is_same_v<double, T>) {
        return _feature->GetFieldAsDouble(name.data());
    } else if constexpr (std::is_same_v<float, T>) {
        return static_cast<float>(_feature->GetFieldAsDouble(name.data()));
    } else if constexpr (std::is_same_v<int32_t, T>) {
        return _feature->GetFieldAsInteger(name.data());
    } else if constexpr (std::is_same_v<int64_t, T>) {
        return _feature->GetFieldAsInteger64(name.data());
    } else if constexpr (std::is_same_v<std::string_view, T>) {
        return std::string_view(_feature->GetFieldAsString(name.data()));
    } else {
        throw std::invalid_argument("Invalid field type");
    }
}

// template instantiations to avoid linker errors
template double Feature::getFieldAs<double>(int index) const;
template float Feature::getFieldAs<float>(int index) const;
template int32_t Feature::getFieldAs<int32_t>(int index) const;
template int64_t Feature::getFieldAs<int64_t>(int index) const;
template std::string_view Feature::getFieldAs<std::string_view>(int index) const;

template double Feature::getFieldAs<double>(std::string_view index) const;
template float Feature::getFieldAs<float>(std::string_view index) const;
template int32_t Feature::getFieldAs<int32_t>(std::string_view index) const;
template int64_t Feature::getFieldAs<int64_t>(std::string_view index) const;
template std::string_view Feature::getFieldAs<std::string_view>(std::string_view index) const;

bool Feature::operator==(const Feature& other) const
{
    if (_feature && other._feature) {
        return _feature->Equal(other._feature);
    } else if (!_feature && !other._feature) {
        return true;
    }

    return false;
}

Layer::Layer(OGRLayer* layer)
: _layer(layer)
{
    if (_layer) {
        _layer->Reference();
    }
}

Layer::Layer(const Layer& other)
{
    _layer = other._layer;
    _layer->Reference();
}

Layer::Layer(Layer&& other)
: _layer(other._layer)
{
    other._layer = nullptr;
}

Layer::~Layer()
{
    if (_layer) {
        _layer->Dereference();
    }
}

int64_t Layer::featureCount() const
{
    return _layer->GetFeatureCount();
}

Feature Layer::feature(int64_t index) const
{
    assert(index < _layer->GetFeatureCount());
    return Feature(checkPointer(_layer->GetFeature(index), "Failed to get feature from layer"));
}

int Layer::fieldIndex(std::string_view name) const
{
    return _layer->FindFieldIndex(name.data(), 1 /*exact match*/);
}

void Layer::setSpatialFilter(Point<double> point)
{
    OGRPoint p(point.x, point.y);
    _layer->SetSpatialFilter(&p);
}

void Layer::createField(FieldDefinition& field)
{
    checkError(_layer->CreateField(field.get()), "Failed to create layer field");
}

void Layer::createFeature(Feature& feature)
{
    checkError(_layer->CreateFeature(feature.get()), "Failed to create layer feature");
}

FeatureDefinition Layer::layerDefinition() const
{
    return FeatureDefinition(checkPointer(_layer->GetLayerDefn(), "Failed to obtain layer definition"));
}

const char* Layer::name() const
{
    return _layer->GetName();
}

OGRLayer* Layer::get()
{
    return _layer;
}

const OGRLayer* Layer::get() const
{
    return _layer;
}

LayerIterator::LayerIterator()
: _layer(nullptr)
, _currentFeature(nullptr)
{
}

LayerIterator::LayerIterator(Layer layer)
: _layer(std::move(layer))
, _currentFeature(nullptr)
{
    _layer.get()->ResetReading();
    next();
}

void LayerIterator::next()
{
    _currentFeature = Feature(_layer.get()->GetNextFeature());
}

const Feature& LayerIterator::operator*()
{
    return _currentFeature;
}

const Feature* LayerIterator::operator->()
{
    return &_currentFeature;
}

LayerIterator& LayerIterator::operator++()
{
    next();
    return *this;
}

LayerIterator& LayerIterator::operator=(LayerIterator&& other)
{
    if (this != &other) {
        _layer          = std::move(other._layer);
        _currentFeature = std::move(other._currentFeature);
    }

    return *this;
}

bool LayerIterator::operator==(const LayerIterator& other) const
{
    return _currentFeature == other._currentFeature;
}

bool LayerIterator::operator!=(const LayerIterator& other) const
{
    return !(*this == other);
}

FeatureIterator::FeatureIterator(int fieldCount)
: _currentFieldIndex(fieldCount)
{
}

FeatureIterator::FeatureIterator(const Feature& feature)
: _feature(&feature)
, _fieldCount(feature.fieldCount())
{
    next();
}

void FeatureIterator::next()
{
    if (_currentFieldIndex < _fieldCount) {
        _currentField = _feature->getField(_currentFieldIndex);
    }
}

const Field& FeatureIterator::operator*()
{
    return _currentField;
}

const Field* FeatureIterator::operator->()
{
    return &_currentField;
}

FeatureIterator& FeatureIterator::operator++()
{
    ++_currentFieldIndex;
    next();
    return *this;
}

bool FeatureIterator::operator==(const FeatureIterator& other) const
{
    return _currentFieldIndex == other._currentFieldIndex;
}

bool FeatureIterator::operator!=(const FeatureIterator& other) const
{
    return !(*this == other);
}

FeatureDefinitionIterator::FeatureDefinitionIterator(int fieldCount)
: _currentFieldIndex(fieldCount)
{
}

FeatureDefinitionIterator::FeatureDefinitionIterator(const FeatureDefinition& featureDef)
: _featureDef(&featureDef)
, _fieldCount(featureDef.fieldCount())
{
    next();
}

void FeatureDefinitionIterator::next()
{
    if (_currentFieldIndex < _fieldCount) {
        _currentField = _featureDef->fieldDefinition(_currentFieldIndex);
    }
}

const FieldDefinition& FeatureDefinitionIterator::operator*()
{
    return _currentField;
}

const FieldDefinition* FeatureDefinitionIterator::operator->()
{
    return &_currentField;
}

FeatureDefinitionIterator& FeatureDefinitionIterator::operator++()
{
    ++_currentFieldIndex;
    next();
    return *this;
}

bool FeatureDefinitionIterator::operator==(const FeatureDefinitionIterator& other) const
{
    return _currentFieldIndex == other._currentFieldIndex;
}

bool FeatureDefinitionIterator::operator!=(const FeatureDefinitionIterator& other) const
{
    return !(*this == other);
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

DataSet DataSet::createRaster(const std::string& filePath, const std::vector<std::string>& driverOpts)
{
    return DataSet(checkPointer(create(filePath,
                                    GDAL_OF_READONLY | GDAL_OF_RASTER,
                                    nullptr,
                                    driverOpts),
        "Failed to open raster file"));
}

DataSet DataSet::createRaster(const std::string& filePath, RasterType type, const std::vector<std::string>& driverOpts)
{
    if (type == RasterType::Unknown) {
        type = guessRasterTypeFromFileName(filePath);
        if (type == RasterType::Unknown) {
            throw RuntimeError("Failed to determine raster type for file ('{}')", filePath);
        }
    }

    return DataSet(checkPointer(create(filePath,
                                    GDAL_OF_READONLY | GDAL_OF_RASTER,
                                    nullptr,
                                    driverOpts),
        "Failed to open raster file"));
}

DataSet DataSet::createVector(const std::string& filePath, const std::vector<std::string>& driverOptions)
{
    return DataSet(checkPointer(create(filePath,
                                    GDAL_OF_READONLY | GDAL_OF_VECTOR,
                                    nullptr,
                                    driverOptions),
        "Failed to open vector file"));
}

DataSet DataSet::createVector(const std::string& filePath, VectorType type, const std::vector<std::string>& driverOptions)
{
    if (type == VectorType::Unknown) {
        type = guessVectorTypeFromFileName(filePath);
        if (type == VectorType::Unknown) {
            throw RuntimeError("Failed to determine vector type for file ('{}')", filePath);
        }
    }

    std::array<const char*, 2> drivers{{s_shapeDriverLookup.at(type), nullptr}};

    return DataSet(checkPointer(create(filePath,
                                    GDAL_OF_READONLY | GDAL_OF_VECTOR,
                                    drivers.data(),
                                    driverOptions),
        "Failed to open vector file"));
}

GDALDataset* DataSet::create(const std::string& filePath,
    unsigned int openFlags,
    const char* const* drivers,
    const std::vector<std::string>& driverOpts)
{
    auto options = createOptionsArray(driverOpts);
    return reinterpret_cast<GDALDataset*>(GDALOpenEx(
        filePath.c_str(),
        openFlags,
        drivers,
        options.size() == 1 ? nullptr : options.data(),
        nullptr));
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
    if (_ptr) {
        GDALClose(reinterpret_cast<GDALDatasetH>(_ptr));
    }

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

std::optional<double> DataSet::noDataValue(int bandNr) const
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

void DataSet::setNoDataValue(int bandNr, std::optional<double> value) const
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

Layer DataSet::getLayer(int index)
{
    assert(_ptr);
    return Layer(checkPointer(_ptr->GetLayer(index), "Invalid layer index"));
}

Layer DataSet::createLayer(const std::string& name, const std::vector<std::string>& driverOptions)
{
    assert(_ptr);
    auto options = createOptionsArray(driverOptions);
    return Layer(checkPointer(_ptr->CreateLayer(name.c_str(), nullptr, wkbUnknown, const_cast<char**>(options.data())), "Layer creation failed"));
}

RasterBand DataSet::rasterBand(int bandNr) const
{
    return RasterBand(checkPointer(_ptr->GetRasterBand(bandNr), "Invalid band index"));
}

GDALDataType DataSet::getBandDataType(int bandNr) const
{
    assert(_ptr);
    assert(bandNr > 0);
    return checkPointer(_ptr->GetRasterBand(bandNr), "Invalid band index")->GetRasterDataType();
}

GDALDataset* DataSet::get() const
{
    return _ptr;
}

Driver DataSet::driver()
{
    return Driver(*_ptr->GetDriver());
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

RasterType guessRasterTypeFromFileName(const std::string& filePath)
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
    }

    return RasterType::Unknown;
}

VectorType guessVectorTypeFromFileName(const std::string& filePath)
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
