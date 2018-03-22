#pragma once

#include "infra/point.h"

#include <gdal_priv.h>
#include <ogr_feature.h>
#include <ogr_spatialref.h>

#include <date/date.h>
#include <optional>
#include <variant>

class OGRSimpleCurve;
class OGRFieldDefn;
class OGRPointIterator;
class OGRGeometryCollection;
class OGRMultiLineString;
class OGRLinearRing;

namespace infra::gdal {

using days       = date::days;
using date_point = std::chrono::time_point<std::chrono::system_clock, days>;

class Layer;

class Geometry
{
public:
    enum class Type
    {
        Point,
        Collection,
        Line,
        MultiLine,
        Polygon,
        Unknown
    };

    OGRGeometry* getGeometry() noexcept;
    const OGRGeometry* getGeometry() const noexcept;
    virtual ~Geometry() = default;

    Type type() const;
    template <typename T>
    T asType() const
    {
        return T(static_cast<typename T::WrappedType*>(_geometry));
    }

protected:
    Geometry(OGRGeometry* instance);

private:
    OGRGeometry* _geometry = nullptr;
};

template <typename OGRType>
class GeometryPtr : public Geometry
{
public:
    using WrappedType = OGRType;

    OGRType* ptr() noexcept
    {
        return _ogrPtr;
    }

    const OGRType* ptr() const noexcept
    {
        return _ogrPtr;
    }

    virtual ~GeometryPtr()
    {
        if (_hasOwnerShip) {
            delete _ogrPtr;
        }
    }

    GeometryPtr()
    : Geometry(new OGRType())
    , _hasOwnerShip(true)
    , _ogrPtr(static_cast<OGRType*>(ptr()))
    {
    }

    GeometryPtr(OGRType* instance)
    : Geometry(instance)
    , _hasOwnerShip(false)
    , _ogrPtr(instance)
    {
    }

private:
    bool _hasOwnerShip = false;
    OGRType* _ogrPtr   = nullptr;
};

template <typename WrappedType>
class GeometryCollectionWrapper : public GeometryPtr<WrappedType>
{
public:
    GeometryCollectionWrapper() = default;
    GeometryCollectionWrapper(WrappedType* collection);

    void addGeometry(const Geometry& geometry);

    int size() const;
    Geometry geometry(int index);
};

using GeometryCollection = GeometryCollectionWrapper<OGRGeometryCollection>;

class Line : public GeometryPtr<OGRSimpleCurve>
{
public:
    Line(OGRSimpleCurve* curve);

    int pointCount() const;
    Point<double> pointAt(int index) const;

    Point<double> startPoint();
    Point<double> endPoint();
};

class PointGeometry : public GeometryPtr<OGRPoint>
{
public:
    PointGeometry(OGRPoint* point);

    Point<double> point() const;
};

class LineIterator
{
public:
    LineIterator() = default;
    LineIterator(Line line);
    LineIterator(const LineIterator&) = delete;
    LineIterator(LineIterator&&)      = default;
    ~LineIterator();

    LineIterator& operator++();
    LineIterator& operator=(LineIterator&& other);
    bool operator==(const LineIterator& other) const;
    bool operator!=(const LineIterator& other) const;
    const Point<double>& operator*();
    const Point<double>* operator->();

private:
    void next();

    OGRPointIterator* _iter = nullptr;
    Point<double> _point;
};

LineIterator begin(const Line& line);
LineIterator begin(Line&& line);
LineIterator end(const Line&);

class MultiLine : public GeometryCollectionWrapper<OGRMultiLineString>
{
public:
    MultiLine(OGRMultiLineString* multiLine);

    Line lineAt(int index);
};

class LinearRing : public Line
{
public:
    LinearRing(OGRLinearRing* ring);
};

class Polygon : public GeometryPtr<OGRPolygon>
{
public:
    Polygon(OGRPolygon* poly);

    LinearRing exteriorRing();
    LinearRing interiorRing(int index);
};

using Field = std::variant<int32_t, int64_t, double, std::string_view>;

class FieldDefinition
{
public:
    FieldDefinition() = default;
    FieldDefinition(const char* name, const std::type_info& typeInfo);
    FieldDefinition(OGRFieldDefn* def);
    ~FieldDefinition();
    std::string_view name() const;
    const std::type_info& type() const;

    OGRFieldDefn* get() noexcept;

private:
    bool _hasOwnerShip = false;
    OGRFieldDefn* _def = nullptr;
};

class FeatureDefinition
{
public:
    FeatureDefinition(OGRFeatureDefn* def);
    ~FeatureDefinition();
    std::string_view name() const;

    int fieldCount() const;
    int fieldIndex(std::string_view name) const;
    FieldDefinition fieldDefinition(int index) const;

    OGRFeatureDefn* get() noexcept;

private:
    bool _hasOwnerShip;
    OGRFeatureDefn* _def;
};

class Feature
{
public:
    Feature(Layer& layer);
    explicit Feature(OGRFeature* feature);
    Feature(const Feature&) = delete;
    Feature(Feature&&);
    ~Feature();

    Feature& operator=(const Feature&) = delete;
    Feature& operator                  =(Feature&&);

    OGRFeature* get();
    const OGRFeature* get() const;

    Geometry geometry();
    Geometry geometry() const;

    void setGeometry(const Geometry& geom);

    int fieldCount() const;
    int fieldIndex(std::string_view name) const;
    FieldDefinition fieldDefinition(int index) const;

    Field getField(int index) const noexcept;

    template <typename T>
    T getFieldAs(int index) const;

    template <typename T>
    T getFieldAs(std::string_view name) const;

    template <typename T>
    void setField(std::string_view name, const T& value)
    {
        _feature->SetField(name.data(), value);
    }

    bool operator==(const Feature& other) const;

private:
    OGRFeature* _feature;
};

class Layer
{
public:
    explicit Layer(OGRLayer* layer);
    Layer(const Layer&);
    Layer(Layer&&);
    ~Layer();

    Layer& operator=(Layer&&) = default;

    int64_t featureCount() const;
    Feature feature(int64_t index) const;

    int fieldIndex(std::string_view name) const;
    void setSpatialFilter(Point<double> point);

    void createField(FieldDefinition& field);
    void createFeature(Feature& feature);

    FeatureDefinition layerDefinition() const;

    const char* name() const;
    OGRLayer* get();
    const OGRLayer* get() const;

private:
    OGRLayer* _layer;
};

// Iteration is not thread safe!
// Do not iterate simultaneously from different threads.
class LayerIterator
{
public:
    LayerIterator();
    LayerIterator(Layer layer);
    LayerIterator(const LayerIterator&) = delete;
    LayerIterator(LayerIterator&&)      = default;

    LayerIterator& operator++();
    LayerIterator& operator=(LayerIterator&& other);
    bool operator==(const LayerIterator& other) const;
    bool operator!=(const LayerIterator& other) const;
    const Feature& operator*();
    const Feature* operator->();

private:
    void next();

    Layer _layer;
    Feature _currentFeature;
};

// support for range based for loops
inline LayerIterator begin(Layer& layer)
{
    return LayerIterator(layer);
}

inline LayerIterator begin(Layer&& layer)
{
    return LayerIterator(layer);
}

inline LayerIterator end(const infra::gdal::Layer& /*layer*/)
{
    return LayerIterator();
}

class FeatureIterator
{
public:
    FeatureIterator(int _count);
    FeatureIterator(const Feature& feature);
    FeatureIterator(const FeatureIterator&) = delete;
    FeatureIterator(FeatureIterator&&)      = default;

    FeatureIterator& operator++();
    FeatureIterator& operator=(FeatureIterator&& other) = default;
    bool operator==(const FeatureIterator& other) const;
    bool operator!=(const FeatureIterator& other) const;
    const Field& operator*();
    const Field* operator->();

private:
    void next();

    const Feature* _feature = nullptr;
    int _fieldCount         = 0;
    int _currentFieldIndex  = 0;
    Field _currentField;
};

// support for range based for loops
inline FeatureIterator begin(const Feature& feat)
{
    return FeatureIterator(feat);
}

inline FeatureIterator begin(Feature&& feat)
{
    return FeatureIterator(feat);
}

inline FeatureIterator end(const Feature& feat)
{
    return FeatureIterator(feat.fieldCount());
}

class FeatureDefinitionIterator
{
public:
    FeatureDefinitionIterator(int fieldCount);
    FeatureDefinitionIterator(const FeatureDefinition& featureDef);
    FeatureDefinitionIterator(const FeatureDefinitionIterator&) = delete;
    FeatureDefinitionIterator(FeatureDefinitionIterator&&)      = default;

    FeatureDefinitionIterator& operator++();
    FeatureDefinitionIterator& operator=(FeatureDefinitionIterator&& other) = default;
    bool operator==(const FeatureDefinitionIterator& other) const;
    bool operator!=(const FeatureDefinitionIterator& other) const;
    const FieldDefinition& operator*();
    const FieldDefinition* operator->();

private:
    void next();

    const FeatureDefinition* _featureDef = nullptr;
    int _fieldCount                      = 0;
    int _currentFieldIndex               = 0;
    FieldDefinition _currentField;
};

// support for range based for loops
inline FeatureDefinitionIterator begin(const FeatureDefinition& featDef)
{
    return FeatureDefinitionIterator(featDef);
}

inline FeatureDefinitionIterator begin(FeatureDefinition&& featDef)
{
    return FeatureDefinitionIterator(featDef);
}

inline FeatureDefinitionIterator end(const FeatureDefinition& featDef)
{
    return FeatureDefinitionIterator(featDef.fieldCount());
}
}
