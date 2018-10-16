#pragma once

#include "infra/exception.h"
#include "infra/point.h"
#include "infra/rect.h"

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

namespace inf::gdal {

using days       = date::days;
using date_point = std::chrono::time_point<std::chrono::system_clock, days>;

class Layer;

template <typename GeometryType>
class Owner : public GeometryType
{
public:
    template <typename OgrType>
    Owner(OgrType* ptr)
    : GeometryType(ptr)
    , _owned(true)
    {
    }

    ~Owner()
    {
        if (_owned) {
            delete GeometryType::get();
        }
    }

    // Don't allow copying
    Owner(const Owner<GeometryType>&) = delete;
    Owner& operator=(const Owner<GeometryType>&) = delete;

    // Allow moving
    Owner(Owner<GeometryType>&& other)
    : GeometryType(other)
    {
        _owned       = other._owned;
        other._owned = false;
    }

    Owner& operator=(Owner<GeometryType>&& other)
    {
        GeometryType::operator=(other);

        _owned       = other._owned;
        other._owned = false;
    }

    auto release()
    {
        _owned = false;
        return GeometryType::get();
    }

private:
    bool _owned;
};

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
        MultiPolygon,
        Unknown
    };

    Geometry(OGRGeometry* instance);

    OGRGeometry* get() noexcept;
    const OGRGeometry* get() const noexcept;

    Type type() const;
    std::string_view type_name() const;

    // The returned type does not have ownership of the geometry
    // the geometry instance has to stay alive
    template <typename T>
    T as() const
    {
        assert(_geometry);
        return T(dynamic_cast<typename T::WrappedType*>(_geometry));
    }

    Owner<Geometry> clone() const;

    bool empty() const;
    void clear();

private:
    OGRGeometry* _geometry = nullptr;
};

template <typename OGRType>
class GeometryPtr : public Geometry
{
public:
    using WrappedType = OGRType;

    OGRType* get() noexcept
    {
        return static_cast<OGRType*>(Geometry::get());
    }

    const OGRType* get() const noexcept
    {
        return static_cast<const OGRType*>(Geometry::get());
    }

    GeometryPtr(OGRType* instance)
    : Geometry(instance)
    {
    }
};

template <typename WrappedType>
class GeometryCollectionWrapper : public GeometryPtr<WrappedType>
{
public:
    GeometryCollectionWrapper(WrappedType* collection);

    void add_geometry(const Geometry& geometry);
    void add_geometry(Owner<Geometry> geometry);

    int size() const;
    Geometry geometry(int index);
};

using GeometryCollection = GeometryCollectionWrapper<OGRGeometryCollection>;

class Line : public GeometryPtr<OGRSimpleCurve>
{
public:
    Line(OGRSimpleCurve* curve);

    int point_count() const;
    Point<double> point_at(int index) const;

    Point<double> startpoint();
    Point<double> endpoint();
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
    LineIterator(const Line& line);
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

    Line line_at(int index);
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

    LinearRing exteriorring();
    LinearRing interiorring(int index);
    int interiorring_count();

    GeometryPtr<OGRGeometry> linear_geometry();
    bool has_curve_geometry() const;
};

class MultiPolygon : public GeometryCollectionWrapper<OGRMultiPolygon>
{
public:
    MultiPolygon(OGRMultiPolygon* multiPoly);

    Polygon polygon_at(int index);
};

template <typename GeometryType>
Owner<GeometryType> createGeometry()
{
    return Owner<GeometryType>(new typename GeometryType::WrappedType());
}

using Field = std::variant<int32_t, int64_t, double, std::string_view>;

class FieldDefinitionRef
{
public:
    FieldDefinitionRef() = default;
    FieldDefinitionRef(OGRFieldDefn* def);
    std::string_view name() const;
    const std::type_info& type() const;

    OGRFieldDefn* get() noexcept;

protected:
    OGRFieldDefn* _def = nullptr;
};

class FieldDefinition : public FieldDefinitionRef
{
public:
    FieldDefinition() = default;
    FieldDefinition(const char* name, const std::type_info& typeInfo);
    FieldDefinition(const std::string& name, const std::type_info& typeInfo);
    FieldDefinition(OGRFieldDefn* def);
    ~FieldDefinition();

    FieldDefinition(const FieldDefinition&) = delete;
    FieldDefinition& operator=(const FieldDefinition&) = delete;

    FieldDefinition(FieldDefinition&&);
    FieldDefinition& operator=(FieldDefinition&&);
};

class FeatureDefinitionRef
{
public:
    FeatureDefinitionRef() = default;
    FeatureDefinitionRef(OGRFeatureDefn* def);
    std::string_view name() const;

    int field_count() const;
    int field_index(std::string_view name) const;
    FieldDefinitionRef field_definition(int index) const;

    OGRFeatureDefn* get() noexcept;

private:
    OGRFeatureDefn* _def = nullptr;
};

class Feature
{
public:
    Feature(FeatureDefinitionRef featurDef);
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

    void set_geometry(const Geometry& geom);

    template <typename GeometryType>
    void set_geometry(Owner<GeometryType> geom)
    {
        get()->SetGeometryDirectly(geom.release());
    }

    int field_count() const;
    int field_index(std::string_view name) const;
    FieldDefinitionRef field_definition(int index) const;

    Field field(int index) const noexcept;

    template <typename T>
    T field_as(int index) const;

    template <typename T>
    T field_as(std::string_view name) const;

    template <typename T>
    void set_field(const std::string& name, const T& value)
    {
        _feature->SetField(name.c_str(), value);
    }

    template <typename T>
    void set_field(const char* name, const T& value)
    {
        _feature->SetField(name, value);
    }

    template <typename T>
    void set_field(int index, const T& value)
    {
        _feature->SetField(index, value);
    }

    bool operator==(const Feature& other) const;

private:
    OGRFeature* _feature;
};

template <>
inline void Feature::set_field<std::string>(int index, const std::string& value)
{
    _feature->SetField(index, value.c_str());
}

/*
 * Data Hierarchy
 * DataSet:
 *  - Layer [*]
 *      - FieldDefinition [1]
 *      - Feature [*]
 *          - Geometry[*]
 */

class Layer
{
public:
    explicit Layer(OGRLayer* layer);
    Layer(const Layer&);
    Layer(Layer&&);
    ~Layer();

    Layer& operator=(Layer&&) = default;

    int64_t feature_count() const;
    Feature feature(int64_t index) const;

    int field_index(std::string_view name) const;
    void set_spatial_filter(Point<double> point);

    void create_field(FieldDefinition& field);
    void create_feature(Feature& feature);

    FeatureDefinitionRef layer_definition() const;

    const char* name() const;
    Rect<double> extent() const;

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

inline LayerIterator end(const inf::gdal::Layer& /*layer*/)
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
    return FeatureIterator(feat.field_count());
}

class FeatureDefinitionIterator
{
public:
    FeatureDefinitionIterator(int fieldCount);
    FeatureDefinitionIterator(FeatureDefinitionRef featureDef);
    FeatureDefinitionIterator(const FeatureDefinitionIterator&) = delete;
    FeatureDefinitionIterator(FeatureDefinitionIterator&&)      = default;

    FeatureDefinitionIterator& operator++();
    FeatureDefinitionIterator& operator=(FeatureDefinitionIterator&& other) = default;
    bool operator==(const FeatureDefinitionIterator& other) const;
    bool operator!=(const FeatureDefinitionIterator& other) const;
    const FieldDefinitionRef& operator*();
    const FieldDefinitionRef* operator->();

private:
    void next();

    FeatureDefinitionRef _featureDef;
    int _fieldCount        = 0;
    int _currentFieldIndex = 0;
    FieldDefinitionRef _currentField;
};

// support for range based for loops
inline FeatureDefinitionIterator begin(FeatureDefinitionRef featDef)
{
    return FeatureDefinitionIterator(featDef);
}

inline FeatureDefinitionIterator end(FeatureDefinitionRef featDef)
{
    return FeatureDefinitionIterator(featDef.field_count());
}

MultiLine forceToMultiLine(Geometry& geom);
}
