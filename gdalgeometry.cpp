#include "infra/gdalgeometry.h"
#include "infra/exception.h"
#include "infra/internal/gdalinternal.h"
#include "infra/log.h"

#include <ogrsf_frmts.h>

namespace infra::gdal {

using namespace std::string_literals;

OGRGeometry* Geometry::get() noexcept
{
    return _geometry;
}

const OGRGeometry* Geometry::get() const noexcept
{
    return _geometry;
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

std::string_view Geometry::typeName() const
{
    return _geometry->getGeometryName();
}

Owner<Geometry> Geometry::clone() const
{
    return Owner<Geometry>(_geometry->clone());
}

bool Geometry::empty() const
{
    return _geometry->IsEmpty();
}

void Geometry::clear()
{
    _geometry->empty();
}

Geometry::Geometry(OGRGeometry* instance)
: _geometry(instance)
{
}

template <typename WrappedType>
GeometryCollectionWrapper<WrappedType>::GeometryCollectionWrapper(WrappedType* collection)
: GeometryPtr<WrappedType>(collection)
{
}

template <typename WrappedType>
void GeometryCollectionWrapper<WrappedType>::addGeometry(const Geometry& geometry)
{
    // clones the geometry
    this->get()->addGeometry(geometry.get());
}

template <typename WrappedType>
void GeometryCollectionWrapper<WrappedType>::addGeometry(Owner<Geometry> geometry)
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
    assert(curve);
}

int Line::pointCount() const
{
    return get()->getNumPoints();
}

Point<double> Line::pointAt(int index) const
{
    OGRPoint p;
    get()->getPoint(index, &p);
    return Point<double>(p.getX(), p.getY());
}

Point<double> Line::startPoint()
{
    OGRPoint point;
    get()->StartPoint(&point);
    return Point<double>(point.getX(), point.getY());
}

Point<double> Line::endPoint()
{
    OGRPoint point;
    get()->EndPoint(&point);
    return Point<double>(point.getX(), point.getY());
}

LineIterator::LineIterator(const Line& line)
: _iter(line.get()->getPointIterator())
{
    next();
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

Line MultiLine::lineAt(int index)
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

LinearRing Polygon::exteriorRing()
{
    return LinearRing(get()->getExteriorRing());
}

LinearRing Polygon::interiorRing(int index)
{
    return LinearRing(get()->getInteriorRing(index));
}

int Polygon::interiorRingCount()
{
    return get()->getNumInteriorRings();
}

GeometryPtr<OGRGeometry> Polygon::getLinearGeometry()
{
    return GeometryPtr<OGRGeometry>(get()->getLinearGeometry());
}

bool Polygon::hasCurveGeometry() const
{
    return get()->hasCurveGeometry();
}

MultiPolygon::MultiPolygon(OGRMultiPolygon* multiLine)
: GeometryCollectionWrapper(multiLine)
{
}

Polygon MultiPolygon::polygonAt(int index)
{
    return geometry(index).as<Polygon>();
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

FieldDefinitionRef::FieldDefinitionRef(OGRFieldDefn* def)
: _def(def)
{
    assert(def);
}

std::string_view FieldDefinitionRef::name() const
{
    return std::string_view(_def->GetNameRef());
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

OGRFieldDefn* FieldDefinitionRef::get() noexcept
{
    return _def;
}

FieldDefinition::FieldDefinition(const char* name, const std::type_info& typeInfo)
: FieldDefinitionRef(new OGRFieldDefn(name, fieldTypeFromTypeInfo(typeInfo)))
{
}

FieldDefinition::FieldDefinition(OGRFieldDefn* def)
: FieldDefinitionRef(def)
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
    _def       = other._def;
    other._def = nullptr;
    return *this;
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

int FeatureDefinitionRef::fieldCount() const
{
    return _def->GetFieldCount();
}

int FeatureDefinitionRef::fieldIndex(std::string_view name) const
{
    return _def->GetFieldIndex(name.data());
}

FieldDefinitionRef FeatureDefinitionRef::fieldDefinition(int index) const
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
    //OGRFeature::DestroyFeature(_feature);
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

void Feature::setGeometry(const Geometry& geom)
{
    checkError(_feature->SetGeometry(geom.get()), "Failed to set geometry");
}

int Feature::fieldCount() const
{
    return _feature->GetFieldCount();
}

int Feature::fieldIndex(std::string_view name) const
{
    return _feature->GetFieldIndex(name.data());
}

FieldDefinitionRef Feature::fieldDefinition(int index) const
{
    return FieldDefinitionRef(checkPointer(_feature->GetFieldDefnRef(index), "Invalid field definition index"));
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

FeatureDefinitionRef Layer::layerDefinition() const
{
    return FeatureDefinitionRef(checkPointer(_layer->GetLayerDefn(), "Failed to obtain layer definition"));
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

FeatureDefinitionIterator::FeatureDefinitionIterator(FeatureDefinitionRef featureDef)
: _featureDef(featureDef)
, _fieldCount(featureDef.fieldCount())
{
    next();
}

void FeatureDefinitionIterator::next()
{
    if (_currentFieldIndex < _fieldCount) {
        _currentField = _featureDef.fieldDefinition(_currentFieldIndex);
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
