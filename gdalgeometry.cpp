#include "infra/gdalgeometry.h"
#include "infra/cast.h"
#include "infra/exception.h"
#include "infra/gdal-private.h"
#include "infra/gdal.h"
#include "infra/string.h"

#include <ogrsf_frmts.h>

namespace inf::gdal {

using namespace std::string_literals;

Geometry::Geometry(OGRGeometry* instance)
: _geometry(instance)
{
}

OGRGeometry* Geometry::get() noexcept
{
    return _geometry;
}

const OGRGeometry* Geometry::get() const noexcept
{
    return _geometry;
}

Geometry::operator bool() const noexcept
{
    return _geometry != nullptr;
}

Geometry::Type Geometry::type() const
{
    switch (wkbFlatten(_geometry->getGeometryType())) {
    case wkbPoint:
        return Geometry::Type::Point;
    case wkbLineString:
        return Geometry::Type::Line;
    case wkbPolygon:
        return Geometry::Type::Polygon;
    case wkbMultiPolygon:
        return Geometry::Type::MultiPolygon;
    case wkbMultiLineString:
        return Geometry::Type::MultiLine;
    default:
        throw RuntimeError("Unsupported geometry type ({})", wkbFlatten(_geometry->getGeometryType()));
    }
}

std::string_view Geometry::type_name() const
{
    return _geometry->getGeometryName();
}

Owner<Geometry> Geometry::clone() const
{
    return Owner<Geometry>(_geometry->clone());
}

bool Geometry::empty() const
{
    return !_geometry || _geometry->IsEmpty();
}

void Geometry::clear()
{
    _geometry->empty();
}

bool Geometry::is_simple() const
{
    return _geometry->IsSimple();
}

Owner<Geometry> Geometry::simplify(double tolerance) const
{
    return Owner<Geometry>(checkPointer(_geometry->Simplify(tolerance), "Failed to simplify geometry"));
}

Owner<Geometry> Geometry::simplify_preserve_topology(double tolerance) const
{
    return Owner<Geometry>(checkPointer(_geometry->Simplify(tolerance), "Failed to simplify geometry preserving topology"));
}

void Geometry::transform(CoordinateTransformer& transformer)
{
    checkError(_geometry->transform(transformer.get()), "Failed to transform geometry");
}

Owner<Geometry> Geometry::buffer(double distance) const
{
    return Owner<Geometry>(checkPointer(_geometry->Buffer(distance), "Failed to buffer geometry"));
}

Owner<Geometry> Geometry::buffer(double distance, int numQuadSegments) const
{
    return Owner<Geometry>(checkPointer(_geometry->Buffer(distance, numQuadSegments), "Failed to buffer geometry"));
}

Owner<Geometry> Geometry::intersection(const Geometry& other) const
{
    return Owner<Geometry>(checkPointer(_geometry->Intersection(other.get()), "Failed to get geometry intersection"));
}

std::optional<double> Geometry::area() const
{
    if (auto* surface = _geometry->toSurface(); surface != nullptr) {
        return surface->get_Area();
    }

    return {};
}

double Geometry::distance(const Point<double>& point) const
{
    OGRPoint p(point.x, point.y);
    return _geometry->Distance(&p);
}

double Geometry::distance(const Geometry& other) const
{
    return _geometry->Distance(other.get());
}

template <typename WrappedType>
GeometryCollectionWrapper<WrappedType>::GeometryCollectionWrapper(WrappedType* collection)
: GeometryPtr<WrappedType>(collection)
{
}

template <typename WrappedType>
void GeometryCollectionWrapper<WrappedType>::add_geometry(const Geometry& geometry)
{
    // clones the geometry
    this->get()->addGeometry(geometry.get());
}

template <typename WrappedType>
void GeometryCollectionWrapper<WrappedType>::add_geometry(Owner<Geometry> geometry)
{
    // transfers ownership of the geometry to the collections
    this->get()->addGeometryDirectly(geometry.release());
}

template <typename WrappedType>
int GeometryCollectionWrapper<WrappedType>::size() const
{
    return this->get()->getNumGeometries();
}

template <typename WrappedType>
Geometry GeometryCollectionWrapper<WrappedType>::geometry(int index)
{
    return Geometry(checkPointer(this->get()->getGeometryRef(index), "No geometry present"));
}

Line::Line(OGRSimpleCurve* curve)
: GeometryPtr(curve)
{
}

int Line::point_count() const
{
    return get()->getNumPoints();
}

Point<double> Line::point_at(int index) const
{
    OGRPoint p;
    get()->getPoint(index, &p);
    return Point<double>(p.getX(), p.getY());
}

Point<double> Line::startpoint()
{
    OGRPoint point;
    get()->StartPoint(&point);
    return Point<double>(point.getX(), point.getY());
}

Point<double> Line::endpoint()
{
    OGRPoint point;
    get()->EndPoint(&point);
    return Point<double>(point.getX(), point.getY());
}

LineIterator::LineIterator(const Line& line)
{
    if (line) {
        _iter = line.get()->getPointIterator();
        next();
    }
}

LineIterator::~LineIterator()
{
    OGRPointIterator::destroy(_iter);
}

LineIterator begin(const Line& line)
{
    return LineIterator(line);
}

LineIterator begin(Line&& line)
{
    return LineIterator(line);
}

LineIterator end(const Line&)
{
    return LineIterator();
}

MultiLine forceToMultiLine(Geometry& geom)
{
    return MultiLine(static_cast<OGRMultiLineString*>(OGRGeometryFactory::forceToMultiLineString(geom.get())));
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

Owner<PointGeometry> PointGeometry::from_point(Point<double> p)
{
    return Owner<PointGeometry>(new OGRPoint(p.x, p.y));
}

PointGeometry::PointGeometry(OGRPoint* point)
: GeometryPtr(point)
{
}

Point<double> PointGeometry::point() const
{
    return Point<double>(get()->getX(), get()->getY());
}

MultiLine::MultiLine(OGRMultiLineString* multiLine)
: GeometryCollectionWrapper(multiLine)
{
    assert(multiLine);
}

Line MultiLine::line_at(int index)
{
    return geometry(index).as<Line>();
}

LinearRing::LinearRing(OGRLinearRing* ring)
: Line(ring)
{
}

Polygon::Polygon(OGRPolygon* poly)
: GeometryPtr(poly)
{
}

LinearRing Polygon::exteriorring()
{
    return LinearRing(get()->getExteriorRing());
}

LinearRing Polygon::interiorring(int index)
{
    return LinearRing(get()->getInteriorRing(index));
}

int Polygon::interiorring_count()
{
    return get()->getNumInteriorRings();
}

GeometryPtr<OGRGeometry> Polygon::linear_geometry()
{
    return GeometryPtr<OGRGeometry>(get()->getLinearGeometry());
}

bool Polygon::has_curve_geometry() const
{
    return get()->hasCurveGeometry();
}

MultiPolygon::MultiPolygon(OGRMultiPolygon* multiLine)
: GeometryCollectionWrapper(multiLine)
{
}

Polygon MultiPolygon::polygon_at(int index)
{
    return geometry(index).as<Polygon>();
}

static OGRFieldType field_type_from_type_info(const std::type_info& typeInfo)
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

FieldDefinitionRef::FieldDefinitionRef(OGRFieldDefn* def)
: _def(def)
{
    assert(def);
}

const char* FieldDefinitionRef::name() const
{
    return _def->GetNameRef();
}

const std::type_info& FieldDefinitionRef::type() const
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
    case OFTDateTime:
        return typeid(time_point);
    case OFTTime:
    case OFTIntegerList:
    case OFTRealList:
    case OFTStringList:
    case OFTWideString:
    case OFTBinary:
    case OFTInteger64List:
    default:
        throw std::runtime_error("Type not implemented");
    }
}

OGRFieldDefn* FieldDefinitionRef::get() noexcept
{
    return _def;
}

FieldDefinition::FieldDefinition(const char* name, const std::type_info& typeInfo)
: FieldDefinitionRef(new OGRFieldDefn(name, field_type_from_type_info(typeInfo)))
{
}

FieldDefinition::FieldDefinition(const std::string& name, const std::type_info& typeInfo)
: FieldDefinition(name.c_str(), typeInfo)
{
}

FieldDefinition::FieldDefinition(OGRFieldDefn* def)
: FieldDefinitionRef(def)
{
}

FieldDefinition::FieldDefinition(FieldDefinitionRef def)
: FieldDefinitionRef(new OGRFieldDefn(def.get()))
{
}

FieldDefinition::~FieldDefinition()
{
    delete get();
}

FieldDefinition::FieldDefinition(FieldDefinition&& other)
: FieldDefinitionRef(other._def)
{
    other._def = nullptr;
}

FieldDefinition& FieldDefinition::operator=(FieldDefinition&& other)
{
    delete get();
    _def       = other._def;
    other._def = nullptr;
    return *this;
}

void FieldDefinition::set_width(int width)
{
    _def->SetWidth(width);
}

int FieldDefinition::width()
{
    return _def->GetWidth();
}

FeatureDefinitionRef::FeatureDefinitionRef(OGRFeatureDefn* def)
: _def(def)
{
    assert(def);
}

std::string_view FeatureDefinitionRef::name() const
{
    return std::string_view(_def->GetName());
}

int FeatureDefinitionRef::field_count() const
{
    return _def->GetFieldCount();
}

int FeatureDefinitionRef::field_index(const char* name) const noexcept
{
    return _def->GetFieldIndex(name);
}

int FeatureDefinitionRef::field_index(const std::string& name) const noexcept
{
    return field_index(name.c_str());
}

int FeatureDefinitionRef::required_field_index(const char* name) const
{
    int index = _def->GetFieldIndex(name);
    if (index < 0) {
        throw RuntimeError("Field not present: {}", name);
    }

    return index;
}

int FeatureDefinitionRef::required_field_index(const std::string& name) const
{
    return required_field_index(name.c_str());
}

FieldDefinitionRef FeatureDefinitionRef::field_definition(int index) const
{
    return FieldDefinitionRef(checkPointer(_def->GetFieldDefn(index), "Failed to obtain field definition"));
}

OGRFeatureDefn* FeatureDefinitionRef::get() noexcept
{
    return _def;
}

Feature::Feature(FeatureDefinitionRef featurDef)
: _feature(OGRFeature::CreateFeature(featurDef.get()))
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
    return Geometry(checkPointer(_feature->GetGeometryRef(), "No geometry present"));
}

Geometry Feature::geometry() const
{
    return const_cast<Feature*>(this)->geometry();
}

bool Feature::has_geometry() const noexcept
{
    return _feature->GetGeometryRef() != nullptr;
}

void Feature::set_geometry(const Geometry& geom)
{
    checkError(_feature->SetGeometry(geom.get()), "Failed to set geometry");
}

int Feature::field_count() const
{
    return _feature->GetFieldCount();
}

int Feature::field_index(const char* name) const
{
    return _feature->GetFieldIndex(name);
}

int Feature::field_index(const std::string& name) const
{
    return field_index(name.c_str());
}

FieldDefinitionRef Feature::field_definition(int index) const
{
    return FieldDefinitionRef(checkPointer(_feature->GetFieldDefnRef(index), "Invalid field definition index"));
}

FeatureDefinitionRef Feature::feature_definition() const
{
    return FeatureDefinitionRef(checkPointer(_feature->GetDefnRef(), "Failed to obtain feature definition"));
}

Field Feature::field(int index) const noexcept
{
    auto& type = field_definition(index).type();
    if (type == typeid(double)) {
        return Field(field_as<double>(index));
    } else if (type == typeid(int32_t)) {
        return Field(field_as<int32_t>(index));
    } else if (type == typeid(int64_t)) {
        return Field(field_as<int64_t>(index));
    } else if (type == typeid(std::string_view)) {
        return Field(field_as<std::string_view>(index));
    }

    return Field();
}

template <typename T>
T Feature::field_as(int index) const
{
    if constexpr (std::is_same_v<double, T>) {
        return _feature->GetFieldAsDouble(index);
    } else if constexpr (std::is_same_v<float, T>) {
        return static_cast<float>(_feature->GetFieldAsDouble(index));
    } else if constexpr (std::is_same_v<int32_t, T>) {
        return _feature->GetFieldAsInteger(index);
    } else if constexpr (std::is_same_v<int64_t, T>) {
        return _feature->GetFieldAsInteger64(index);
    } else if constexpr (std::is_same_v<std::string, T>) {
        return std::string(_feature->GetFieldAsString(index));
    } else if constexpr (std::is_same_v<std::string_view, T>) {
        return std::string_view(_feature->GetFieldAsString(index));
    } else if constexpr (std::is_same_v<time_point, T>) {
        int year, month, day, hour, minute, timezoneFlag;
        float second;
        if (_feature->GetFieldAsDateTime(index, &year, &month, &day, &hour, &minute, &second, &timezoneFlag) == FALSE) {
            throw RuntimeError("Failed to get field as time point {}", _feature->GetFieldAsString(index));
        }

        auto date      = date::year_month_day(date::year(year), date::month(month), date::day(day));
        auto timePoint = std::chrono::time_point_cast<std::chrono::milliseconds>(static_cast<date::sys_days>(date));
        timePoint += std::chrono::hours(hour) + std::chrono::minutes(minute) + std::chrono::milliseconds(inf::truncate<int>(second * 1000));
        return timePoint;
    } else if constexpr (std::is_same_v<date_point, T>) {
        int year, month, day, hour, minute, timezoneFlag;
        float second;
        if (_feature->GetFieldAsDateTime(index, &year, &month, &day, &hour, &minute, &second, &timezoneFlag) == FALSE) {
            throw RuntimeError("Failed to get field as date point: {}", _feature->GetFieldAsString(index));
        }

        auto date = date::year_month_day(date::year(year), date::month(month), date::day(day));
        return static_cast<date::sys_days>(date);
    } else {
        throw InvalidArgument("Invalid field type");
    }
}

template <typename T>
std::optional<T> Feature::opt_field_as(int index) const
{
    static_assert(!(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>), "optional field should not be used for strings");

    if (!_feature->IsFieldSetAndNotNull(index)) {
        return {};
    }

    return field_as<T>(index);
}

template <typename T>
T Feature::field_as(std::string_view name) const
{
    if constexpr (std::is_same_v<double, T>) {
        return _feature->GetFieldAsDouble(name.data());
    } else if constexpr (std::is_same_v<float, T>) {
        return static_cast<float>(_feature->GetFieldAsDouble(name.data()));
    } else if constexpr (std::is_same_v<int32_t, T>) {
        return _feature->GetFieldAsInteger(name.data());
    } else if constexpr (std::is_same_v<int64_t, T>) {
        return _feature->GetFieldAsInteger64(name.data());
    } else if constexpr (std::is_same_v<std::string, T>) {
        return std::string(_feature->GetFieldAsString(name.data()));
    } else if constexpr (std::is_same_v<std::string_view, T>) {
        return std::string_view(_feature->GetFieldAsString(name.data()));
    } else if constexpr (std::is_same_v<time_point, T>) {
        return field_as<T>(_feature->GetFieldIndex(name.data()));
    } else if constexpr (std::is_same_v<date_point, T>) {
        return field_as<T>(_feature->GetFieldIndex(name.data()));
    } else {
        throw std::invalid_argument("Invalid field type");
    }
}

// template instantiations to avoid linker errors
template double Feature::field_as<double>(int index) const;
template float Feature::field_as<float>(int index) const;
template int32_t Feature::field_as<int32_t>(int index) const;
template int64_t Feature::field_as<int64_t>(int index) const;
template std::string Feature::field_as<std::string>(int index) const;
template std::string_view Feature::field_as<std::string_view>(int index) const;
template time_point Feature::field_as<time_point>(int index) const;
template date_point Feature::field_as<date_point>(int index) const;

template std::optional<double> Feature::opt_field_as<double>(int index) const;
template std::optional<float> Feature::opt_field_as<float>(int index) const;
template std::optional<int32_t> Feature::opt_field_as<int32_t>(int index) const;
template std::optional<int64_t> Feature::opt_field_as<int64_t>(int index) const;
template std::optional<time_point> Feature::opt_field_as<time_point>(int index) const;
template std::optional<date_point> Feature::opt_field_as<date_point>(int index) const;

template double Feature::field_as<double>(std::string_view index) const;
template float Feature::field_as<float>(std::string_view index) const;
template int32_t Feature::field_as<int32_t>(std::string_view index) const;
template int64_t Feature::field_as<int64_t>(std::string_view index) const;
template std::string Feature::field_as<std::string>(std::string_view index) const;
template std::string_view Feature::field_as<std::string_view>(std::string_view index) const;
template time_point Feature::field_as<time_point>(std::string_view index) const;
template date_point Feature::field_as<date_point>(std::string_view index) const;

void Feature::set_field(int index, const Field& field)
{
    std::visit([this, index](const auto& val) {
        set_field<std::decay_t<decltype(val)>>(index, val);
    },
               field);
}

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

std::optional<int32_t> Layer::epsg() const
{
    if (auto* spatialRef = _layer->GetSpatialRef(); spatialRef != nullptr) {
        if (auto* epsg = _layer->GetSpatialRef()->GetAuthorityCode("PROJCS"); epsg != nullptr) {
            return str::to_int32(epsg);
        }
    }

    return std::optional<int32_t>();
}

void Layer::set_projection_from_epsg(int32_t epsg)
{
    SpatialReference spatialRef(epsg);
}

void Layer::set_ignored_fields(const std::vector<std::string>& fieldnames)
{
    auto fields = create_string_array(fieldnames);
    checkError(_layer->SetIgnoredFields(fields.data()), "Failed to ignore layer fields");
}

int64_t Layer::feature_count() const
{
    return _layer->GetFeatureCount();
}

Feature Layer::feature(int64_t index) const
{
    assert(index != OGRNullFID);
    return Feature(checkPointer(_layer->GetFeature(index), "Failed to get feature from layer"));
}

int Layer::field_index(const char* name) const
{
    return _layer->FindFieldIndex(name, 1 /*exact match*/);
}

int Layer::field_index(const std::string& name) const
{
    return field_index(name.c_str());
}

void Layer::set_spatial_filter(Point<double> point)
{
    OGRPoint p(point.x, point.y);
    _layer->SetSpatialFilter(&p);
}

void Layer::set_spatial_filter(Geometry& geometry)
{
    _layer->SetSpatialFilter(geometry.get());
}

void Layer::set_attribute_filter(const char* name)
{
    checkError(_layer->SetAttributeFilter(name), "Failed to set attribute filter");
}

void Layer::set_attribute_filter(const std::string& name)
{
    set_attribute_filter(name.c_str());
}

void Layer::create_field(FieldDefinition& field)
{
    checkError(_layer->CreateField(field.get()), "Failed to create layer field");
}

void Layer::create_feature(Feature& feature)
{
    checkError(_layer->CreateFeature(feature.get()), "Failed to create layer feature");
}

FeatureDefinitionRef Layer::layer_definition() const
{
    return FeatureDefinitionRef(checkPointer(_layer->GetLayerDefn(), "Failed to obtain layer definition"));
}

const char* Layer::name() const
{
    return _layer->GetName();
}

Rect<double> Layer::extent() const
{
    OGREnvelope env;
    checkError(_layer->GetExtent(&env, TRUE), "Failed to get layer extent");

    Rect<double> result;
    result.topLeft.x     = env.MinX;
    result.topLeft.y     = env.MaxY;
    result.bottomRight.x = env.MaxX;
    result.bottomRight.y = env.MinY;

    return result;
}

OGRLayer* Layer::get()
{
    return _layer;
}

const OGRLayer* Layer::get() const
{
    return _layer;
}

OGRLayerH Layer::handle()
{
    return OGRLayer::ToHandle(_layer);
}

const void* Layer::handle() const
{
    return OGRLayer::ToHandle(_layer);
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
, _fieldCount(feature.field_count())
{
    next();
}

void FeatureIterator::next()
{
    if (_currentFieldIndex < _fieldCount) {
        _currentField = _feature->field(_currentFieldIndex);
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

FeatureDefinitionIterator::FeatureDefinitionIterator(FeatureDefinitionRef featureDef)
: _featureDef(featureDef)
, _fieldCount(featureDef.field_count())
{
    next();
}

void FeatureDefinitionIterator::next()
{
    if (_currentFieldIndex < _fieldCount) {
        _currentField = _featureDef.field_definition(_currentFieldIndex);
    }
}

const FieldDefinitionRef& FeatureDefinitionIterator::operator*()
{
    return _currentField;
}

const FieldDefinitionRef* FeatureDefinitionIterator::operator->()
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

template class GeometryCollectionWrapper<OGRMultiPolygon>;
template class GeometryCollectionWrapper<OGRMultiLineString>;
template class GeometryCollectionWrapper<OGRGeometryCollection>;
}
