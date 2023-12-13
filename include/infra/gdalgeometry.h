#pragma once

#include "infra/conversion.h"
#include "infra/coordinate.h"
#include "infra/exception.h"
#include "infra/gdal-private.h"
#include "infra/gdalspatialreference.h"
#include "infra/point.h"
#include "infra/progressinfo.h"
#include "infra/rect.h"
#include "infra/span.h"

#include <gdal_priv.h>
#include <ogr_feature.h>
#include <ogr_spatialref.h>
#include <ogrsf_frmts.h>

#include <chrono>
#ifndef HAVE_CPP20_CHRONO
#include <date/date.h>
#endif
#include <cassert>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <utility>

class OGRSimpleCurve;
class OGRFieldDefn;
class OGRPointIterator;
class OGRGeometryCollection;
class OGRMultiLineString;
class OGRLinearRing;

namespace inf::gdal {

#ifdef HAVE_CPP20_CHRONO
using days     = std::chrono::days;
using sys_days = std::chrono::sys_days;
#else
using days     = date::days;
using sys_days = date::sys_days;
#endif
using date_point = std::chrono::time_point<std::chrono::system_clock, days>;
using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;

namespace Geometry {
enum class Type
{
    Point,
    MultiPoint,
    Collection,
    Line,
    MultiLine,
    Polygon,
    MultiPolygon,
    CurvePolygon,
    LinearRing,
    GeometryCollection,
    MultiSurface,
    MultiCurve,
    None,
    Unknown
};
}

Geometry::Type geometry_type_from_gdal(OGRwkbGeometryType type);

template <typename GeometryType>
class Owner : public GeometryType
{
public:
    Owner() noexcept
    : GeometryType(nullptr)
    , _owned(false)
    {
    }

    Owner(typename GeometryType::wrapped_type* ptr)
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
    Owner(const Owner<GeometryType>&)            = delete;
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
        return *this;
    }

    auto release()
    {
        _owned = false;
        return GeometryType::get();
    }

private:
    bool _owned;
};

class Envelope
{
public:
    Envelope() = default;
    Envelope(double minX, double maxX, double minY, double maxY) noexcept;
    Envelope(Point<int64_t> topleft, Point<int64_t> bottomRight) noexcept;
    Envelope(Point<double> topleft, Point<double> bottomRight) noexcept;

    OGREnvelope* get() noexcept;
    const OGREnvelope* get() const noexcept;

    explicit operator bool() const noexcept;

    void merge(const Envelope& other) noexcept;
    void merge(double x, double y) noexcept;
    void intersect(const Envelope& other) noexcept;

    bool intersects(const Envelope& other) const noexcept;
    bool contains(const Envelope& other) const noexcept;

    Point<double> top_left() const noexcept;
    Point<double> bottom_right() const noexcept;

private:
    OGREnvelope _envelope;
};

template <typename TWrapped>
class GeometryWrapper
{
public:
    using wrapped_type = TWrapped;
    using bare_type    = std::remove_cv_t<wrapped_type>;

    static_assert(std::is_base_of_v<OGRGeometry, bare_type>, "Geometry wrapper should be used with types derived from OGRGeometry");

    GeometryWrapper(TWrapped* instance)
    : _geometry(instance)
    {
    }

    template <typename TWrapper, typename = std::enable_if_t<std::is_base_of_v<OGRGeometry, std::remove_cv_t<typename TWrapper::wrapped_type>>>>
    GeometryWrapper(const Owner<TWrapper>& other)
    : _geometry(other.get())
    {
    }

    template <typename T, typename = std::enable_if_t<(std::is_const_v<TWrapped> == std::is_const_v<T>) || (std::is_const_v<TWrapped> && !std::is_const_v<T>)>>
    GeometryWrapper(GeometryWrapper<T> other)
    : _geometry(other.get())
    {
    }

    template <typename T>
    GeometryWrapper& operator=(const Owner<GeometryWrapper<T>>& other)
    {
        _geometry = other.get();
        return *this;
    }

    TWrapped* get() noexcept
    {
        return _geometry;
    }

    std::add_const_t<TWrapped*> get() const noexcept
    {
        return _geometry;
    }

    explicit operator bool() const noexcept
    {
        return _geometry != nullptr;
    }

    Geometry::Type type() const
    {
        return geometry_type_from_gdal(_geometry->getGeometryType());
    }

    std::string_view type_name() const
    {
        return _geometry->getGeometryName();
    }

    // The returned type does not have ownership of the geometry
    // the geometry instance has to stay alive
    template <typename T>
    T as()
    {
        static_assert(!(std::is_const_v<wrapped_type> && !std::is_const_v<typename T::wrapped_type>), "Attempt to cast away geometry wrapper constness (use CRef type instead)");

        assert(_geometry);
        return T(dynamic_cast<typename T::wrapped_type*>(_geometry));
    }

    Owner<GeometryWrapper<bare_type>> clone() const
    {
        return Owner<GeometryWrapper<bare_type>>(static_cast<bare_type*>(_geometry->clone()));
    }

    template <bool is_const = std::is_const_v<wrapped_type>>
    std::enable_if_t<!is_const, void> clear()
    {
        _geometry->empty();
    }

    bool is_valid() const noexcept
    {
        return _geometry->IsValid() == TRUE;
    }

    Owner<GeometryWrapper<TWrapped>> make_valid()
    {
#if GDAL_VERSION_MAJOR >= 3
        return _geometry->MakeValid();
#endif

        throw RuntimeError("make_valid requires gdal 3 or newer");
    }

    bool is_simple() const
    {
        return _geometry->IsSimple();
    }

    [[nodiscard]] Owner<GeometryWrapper<bare_type>> simplify(double tolerance) const
    {
        return Owner<GeometryWrapper<bare_type>>(static_cast<bare_type*>(check_pointer(_geometry->Simplify(tolerance), "Failed to simplify geometry")));
    }

    [[nodiscard]] Owner<GeometryWrapper<bare_type>> simplify_preserve_topology(double tolerance) const
    {
        return Owner<GeometryWrapper<bare_type>>(static_cast<bare_type*>(check_pointer(_geometry->Simplify(tolerance), "Failed to simplify geometry preserving topology")));
    }

    template <bool is_const = std::is_const_v<wrapped_type>>
    std::enable_if_t<!is_const, void> transform(CoordinateTransformer& transformer)
    {
        check_error(_geometry->transform(transformer.get()), "Failed to transform geometry");
    }

    template <bool is_const = std::is_const_v<wrapped_type>>
    std::enable_if_t<!is_const, void> transform_to(SpatialReference& srs)
    {
        check_error(_geometry->transformTo(srs.get()), "Failed to transform geometry");
    }

    [[nodiscard]] Owner<GeometryWrapper<bare_type>> buffer(double distance) const
    {
        return Owner<GeometryWrapper<bare_type>>(static_cast<bare_type*>(check_pointer(_geometry->Buffer(distance), "Failed to buffer geometry")));
    }

    [[nodiscard]] Owner<GeometryWrapper<bare_type>> buffer(double distance, int numQuadSegments) const
    {
        return Owner<GeometryWrapper<bare_type>>(static_cast<bare_type*>(check_pointer(_geometry->Buffer(distance, numQuadSegments), "Failed to buffer geometry")));
    }

    [[nodiscard]] Owner<GeometryWrapper<bare_type>> intersection(GeometryWrapper<const bare_type> other) const
    {
        return Owner<GeometryWrapper<bare_type>>(static_cast<bare_type*>(check_pointer(_geometry->Intersection(other.get()), "Failed to get geometry intersection")));
    }

    bool intersects(const Point<double>& point) const
    {
        OGRPoint p(point.x, point.y);
        return _geometry->Intersects(&p);
    }

    bool intersects(GeometryWrapper<const bare_type> geom) const
    {
        return _geometry->Intersects(geom.get());
    }

    bool contains(const Point<double>& point) const
    {
        OGRPoint p(point.x, point.y);
        return throw_if_not_supported(_geometry->Contains(&p)) == TRUE;
    }

    bool contains(GeometryWrapper<const bare_type> geom) const
    {
        return throw_if_not_supported(_geometry->Contains(geom.get())) == TRUE;
    }

    bool overlaps(GeometryWrapper<const bare_type> geom) const
    {
        return throw_if_not_supported(_geometry->Overlaps(geom.get()) == TRUE);
    }

    bool within(GeometryWrapper<const bare_type> geom) const
    {
        return throw_if_not_supported(_geometry->Within(geom.get()) == TRUE);
    }

    bool crosses(GeometryWrapper<const bare_type> geom) const
    {
        return throw_if_not_supported(_geometry->Crosses(geom.get()) == TRUE);
    }

    std::optional<double> area() const
    {
        if (auto* surface = _geometry->toSurface(); surface != nullptr) {
            return surface->get_Area();
        }

        return {};
    }

    std::optional<Point<double>> centroid() const
    {
        std::optional<Point<double>> result;
        OGRPoint point;
        if (_geometry->Centroid(&point) == OGRERR_NONE) {
            result = Point<double>(point.getX(), point.getY());
        }

        return result;
    }

    std::optional<Coordinate> centroid_coordinate() const
    {
        if (auto point = centroid(); point.has_value()) {
            return to_coordinate(*point);
        }

        return {};
    }

    double distance(const inf::Point<double>& point) const
    {
        OGRPoint p(point.x, point.y);
        return _geometry->Distance(&p);
    }

    double distance(GeometryWrapper<const bare_type> other) const
    {
        return _geometry->Distance(other.get());
    }

    std::string to_json() const
    {
        std::string result;

        CplPointer<char> json(_geometry->exportToJson());
        if (json) {
            result.assign(json);
        }

        return result;
    }

    Envelope envelope() const
    {
        Envelope env;
        _geometry->getEnvelope(env.get());
        return env;
    }

private:
    wrapped_type* _geometry = nullptr;
};

using GeometryRef  = GeometryWrapper<OGRGeometry>;
using GeometryCRef = GeometryWrapper<const OGRGeometry>;

template <typename WrappedType, typename WrappedCollectionType>
class GeometryCollectionWrapper : public GeometryWrapper<WrappedType>
{
public:
    GeometryCollectionWrapper(WrappedType* collection)
    : GeometryWrapper<WrappedType>(collection)
    {
    }

    void add_geometry(GeometryWrapper<WrappedCollectionType> geometry)
    {
        // clones the geometry
        this->get()->addGeometry(geometry.get());
    }

    void add_geometry(Owner<GeometryWrapper<WrappedCollectionType>> geometry)
    {
        // transfers ownership of the geometry to the collections
        this->get()->addGeometryDirectly(geometry.release());
    }

    int size() const
    {
        return this->get()->getNumGeometries();
    }

    GeometryWrapper<WrappedCollectionType> geometry(int index)
    {
        if constexpr (std::is_const_v<WrappedType>) {
            return GeometryWrapper<WrappedCollectionType>(static_cast<WrappedCollectionType*>(check_pointer(std::as_const(*this).get()->getGeometryRef(index), "No geometry present")));
        } else {
            return GeometryWrapper<WrappedCollectionType>(static_cast<WrappedCollectionType*>(check_pointer(this->get()->getGeometryRef(index), "No geometry present")));
        }
    }
};

using GeometryCollectionRef  = GeometryCollectionWrapper<OGRGeometryCollection, OGRGeometry>;
using GeometryCollectionCRef = GeometryCollectionWrapper<const OGRGeometryCollection, const OGRGeometry>;

template <typename TWrapped>
class LineWrapper : public GeometryWrapper<TWrapped>
{
public:
    LineWrapper(TWrapped* curve)
    : GeometryWrapper<TWrapped>(curve)
    {
    }

    template <typename T, typename = std::enable_if_t<std::is_const_v<TWrapped> && !std::is_const_v<T>>>
    LineWrapper(const LineWrapper<T>& other)
    : GeometryWrapper<TWrapped>(other.get())
    {
    }

    int point_count() const
    {
        return this->get()->getNumPoints();
    }

    Point<double> point_at(int index) const
    {
        OGRPoint p;
        this->get()->getPoint(index, &p);
        return Point<double>(p.getX(), p.getY());
    }

    Point<double> startpoint()
    {
        OGRPoint point;
        this->get()->StartPoint(&point);
        return Point<double>(point.getX(), point.getY());
    }

    Point<double> endpoint()
    {
        OGRPoint point;
        this->get()->EndPoint(&point);
        return Point<double>(point.getX(), point.getY());
    }

    double length() const
    {
        return this->get()->get_Length();
    }

    template <bool is_const = std::is_const_v<TWrapped>>
    std::enable_if_t<!is_const, void> add_point(double x, double y)
    {
        this->get()->addPoint(x, y);
    }

    template <bool is_const = std::is_const_v<TWrapped>>
    std::enable_if_t<!is_const, void> add_point(Point<double> point)
    {
        this->get()->addPoint(point.x, point.y);
    }
};

using LineRef  = LineWrapper<OGRSimpleCurve>;
using LineCRef = LineWrapper<const OGRSimpleCurve>;

template <typename TWrapped>
class PointWrapper : public GeometryWrapper<TWrapped>
{
public:
    using bare_type = std::remove_cv_t<TWrapped>;

    static Owner<PointWrapper<bare_type>> from_point(Point<double> p)
    {
        return Owner<PointWrapper<bare_type>>(new OGRPoint(p.x, p.y));
    }

    PointWrapper(TWrapped* point)
    : GeometryWrapper<TWrapped>(point)
    {
    }

    template <typename T, typename = std::enable_if_t<std::is_const_v<TWrapped> && !std::is_const_v<T>>>
    PointWrapper(const GeometryWrapper<T>& other)
    : GeometryWrapper<TWrapped>(other.get())
    {
    }

    double x() const noexcept
    {
        return this->get()->getX();
    }

    double y() const noexcept
    {
        return this->get()->getY();
    }

    Point<double> point() const
    {
        return Point<double>(this->get()->getX(), this->get()->getY());
    }
};

using PointRef  = PointWrapper<OGRPoint>;
using PointCRef = PointWrapper<const OGRPoint>;

template <typename TLine>
class LineIterator
{
public:
    LineIterator() = default;
    LineIterator(LineWrapper<TLine> line)
    {
        if (line) {
            _iter = line.get()->getPointIterator();
            next();
        }
    }

    LineIterator(const LineIterator&) = delete;
    LineIterator(LineIterator&&)      = default;
    ~LineIterator()
    {
        OGRPointIterator::destroy(_iter);
    }

    LineIterator& operator++()
    {
        next();
        return *this;
    }

    LineIterator& operator=(LineIterator&& other)
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

    bool operator==(const LineIterator& other) const
    {
        return _iter == other._iter;
    }

    bool operator!=(const LineIterator& other) const
    {
        return !(*this == other);
    }

    const Point<double>& operator*()
    {
        return _point;
    }

    const Point<double>* operator->()
    {
        return &_point;
    }

private:
    void next()
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

    OGRPointIterator* _iter = nullptr;
    Point<double> _point;
};

template <typename T>
LineIterator<T> begin(LineWrapper<T> line)
{
    return LineIterator<T>(line);
}

template <typename T>
LineIterator<T> end(LineWrapper<T>)
{
    return LineIterator<T>();
}

template <typename TWrapped, typename TWrappedInner>
class MultiLineWrapper : public GeometryCollectionWrapper<TWrapped, TWrappedInner>
{
public:
    MultiLineWrapper(TWrapped* multiLine)
    : GeometryCollectionWrapper<TWrapped, TWrappedInner>(multiLine)
    {
        assert(multiLine);
    }

    template <bool is_const = std::is_const_v<TWrapped>>
    std::enable_if_t<is_const, LineCRef> line_at(int index)
    {
        return this->geometry(index).template as<LineCRef>();
    }

    template <bool is_const = std::is_const_v<TWrapped>>
    std::enable_if_t<!is_const, LineRef> line_at(int index)
    {
        return this->geometry(index).template as<LineRef>();
    }

    double length() const
    {
        return this->get()->get_Length();
    }
};

using MultiLineRef  = MultiLineWrapper<OGRMultiLineString, OGRLineString>;
using MultiLineCRef = MultiLineWrapper<const OGRMultiLineString, const OGRLineString>;

template <typename TWrapped>
class LinearRingWrapper : public LineWrapper<TWrapped>
{
public:
    LinearRingWrapper(TWrapped* ring)
    : LineWrapper<TWrapped>(ring)
    {
    }

    LinearRingWrapper(const Owner<LinearRingWrapper<std::remove_const_t<TWrapped>>>& ring)
    : LineWrapper<TWrapped>(ring.get())
    {
    }

    bool is_clockwise() const
    {
        return this->get()->isClockwise() == TRUE;
    }
};

using LinearRingRef  = LinearRingWrapper<OGRLinearRing>;
using LinearRingCRef = LinearRingWrapper<const OGRLinearRing>;

template <typename TWrapped>
class PolygonWrapper : public GeometryWrapper<TWrapped>
{
public:
    PolygonWrapper()
    : PolygonWrapper(nullptr)
    {
    }

    PolygonWrapper(TWrapped* poly)
    : GeometryWrapper<TWrapped>(poly)
    {
    }

    template <typename T, typename = std::enable_if_t<std::is_const_v<TWrapped> && !std::is_const_v<T>>>
    PolygonWrapper(const PolygonWrapper<T>& other)
    : GeometryWrapper<TWrapped>(other.get())
    {
    }

    template <bool is_const = std::is_const_v<TWrapped>>
    std::enable_if_t<is_const, LinearRingCRef> exterior_ring()
    {
        return this->get()->getExteriorRing();
    }

    template <bool is_const = std::is_const_v<TWrapped>>
    std::enable_if_t<!is_const, LinearRingRef> exterior_ring()
    {
        return this->get()->getExteriorRing();
    }

    template <bool is_const = std::is_const_v<TWrapped>>
    std::enable_if_t<is_const, LinearRingCRef> interior_ring(int index)
    {
        return this->get()->getInteriorRing(index);
    }

    template <bool is_const = std::is_const_v<TWrapped>>
    std::enable_if_t<!is_const, LinearRingRef> interior_ring(int index)
    {
        return this->get()->getInteriorRing(index);
    }

    int interior_ring_count()
    {
        return this->get()->getNumInteriorRings();
    }

    Owner<GeometryWrapper<OGRGeometry>> linear_geometry()
    {
        return Owner<GeometryWrapper<OGRGeometry>>(this->get()->getLinearGeometry());
    }

    bool has_curve_geometry() const
    {
        return this->get()->hasCurveGeometry();
    }

    void add_ring(LinearRingRef ring)
    {
        this->get()->addRing(ring.get());
    }

    void add_ring(Owner<LinearRingRef> ring)
    {
        this->get()->addRingDirectly(ring.release());
    }
};

using PolygonRef  = PolygonWrapper<OGRPolygon>;
using PolygonCRef = PolygonWrapper<const OGRPolygon>;

template <typename TWrapped, typename TWrappedInner>
class MultiPolygonWrapper : public GeometryCollectionWrapper<TWrapped, TWrappedInner>
{
public:
    MultiPolygonWrapper(TWrapped* multiPoly)
    : GeometryCollectionWrapper<TWrapped, TWrappedInner>(multiPoly)
    {
    }

    template <bool is_const = std::is_const_v<TWrapped>>
    std::enable_if_t<!is_const, PolygonRef> polygon_at(int index)
    {
        return this->geometry(index).template as<PolygonRef>();
    }

    template <bool is_const = std::is_const_v<TWrapped>>
    std::enable_if_t<is_const, PolygonCRef> polygon_at(int index)
    {
        return this->geometry(index).template as<PolygonCRef>();
    }
};

using MultiPolygonRef  = MultiPolygonWrapper<OGRMultiPolygon, OGRPolygon>;
using MultiPolygonCRef = MultiPolygonWrapper<const OGRMultiPolygon, const OGRPolygon>;

using Field = std::variant<int32_t, int64_t, double, std::string_view, time_point>;

class FieldDefinitionRef
{
public:
    FieldDefinitionRef() = default;
    FieldDefinitionRef(OGRFieldDefn* def);
    const char* name() const;
    const std::type_info& type() const;

    OGRFieldDefn* get() noexcept;

protected:
    OGRFieldDefn* _def = nullptr;
};

class FieldDefinition : public FieldDefinitionRef
{
public:
    template <typename T>
    static FieldDefinition create(const char* name)
    {
        return FieldDefinition(name, typeid(T));
    }

    template <typename T>
    static FieldDefinition create(const std::string& name)
    {
        return FieldDefinition(name, typeid(T));
    }

    FieldDefinition() = default;
    FieldDefinition(const char* name, const std::type_info& typeInfo);
    FieldDefinition(const std::string& name, const std::type_info& typeInfo);
    FieldDefinition(OGRFieldDefn* def);
    //! Create a new field definition by cloning the reference
    explicit FieldDefinition(FieldDefinitionRef def);
    ~FieldDefinition();

    FieldDefinition(const FieldDefinition&)            = delete;
    FieldDefinition& operator=(const FieldDefinition&) = delete;

    FieldDefinition(FieldDefinition&&);
    FieldDefinition& operator=(FieldDefinition&&);

    int width();
    void set_width(int width);
};

class FeatureDefinition
{
public:
    FeatureDefinition() = default;
    FeatureDefinition(const FeatureDefinition&);
    FeatureDefinition(const char* name);
    FeatureDefinition(OGRFeatureDefn* def);
    ~FeatureDefinition();
    std::string_view name() const;

    int field_count() const;
    /*! obtain the index of the provided field name, returns -1 if not found */
    int field_index(const char* name) const noexcept;
    int field_index(const std::string& name) const noexcept;
    /*! obtain the index of the provided field name, throws RuntimError if field not present */
    int required_field_index(const char* name) const;
    int required_field_index(const std::string& name) const;
    FieldDefinitionRef field_definition(int index) const;

    OGRFeatureDefn* get() noexcept;

private:
    OGRFeatureDefn* _def = nullptr;
};

class Feature
{
public:
    Feature(FeatureDefinition featurDef);
    explicit Feature(OGRFeature* feature);
    Feature(const Feature&) = delete;
    Feature(Feature&&);
    ~Feature();

    Feature& operator=(const Feature&) = delete;
    Feature& operator=(Feature&&);

    OGRFeature* get();
    const OGRFeature* get() const;

    GeometryRef geometry();
    GeometryCRef geometry() const;
    bool has_geometry() const noexcept;
    template <typename GeometryType>
    void set_geometry(GeometryWrapper<GeometryType> geom)
    {
        check_error(_feature->SetGeometry(geom.get()), "Failed to set geometry");
    }

    template <typename GeometryType>
    void set_geometry(Owner<GeometryType> geom)
    {
        static_assert(!std::is_const_v<typename GeometryType::wrapped_type>, "Set geometry cannot be used with const geometry as it transfers ownership");
        get()->SetGeometryDirectly(geom.release());
    }

    int64_t id() const;

    int field_count() const;
    int field_index(const char* name) const;
    int field_index(const std::string& name) const;
    bool field_is_valid(int index) const noexcept;

    FieldDefinitionRef field_definition(int index) const;
    FeatureDefinition feature_definition() const;

    Field field(int index) const noexcept;
    std::optional<Field> opt_field(int index) const noexcept;

    template <typename T>
    T field_as(int index) const;

    template <typename T>
    std::optional<T> opt_field_as(int index) const;

    template <typename T>
    T field_as(std::string_view name) const;

    void set_field_to_null(int index);

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

    void set_field(int index, const Field& field);

    enum class FieldCopyMode
    {
        Strict,    // the operation will fail if some of the input fields are not present in the output
        Forgiving, // the operation will continue despite lacking output fields matching some of the source fields
    };

    void set_from(const Feature& other, FieldCopyMode mode);
    void set_from(const Feature& other, FieldCopyMode mode, std::span<int32_t> fieldIndexes);
    void set_fields_from(const Feature& other, FieldCopyMode mode, std::span<int32_t> fieldIndexes);

    bool operator==(const Feature& other) const;

private:
    OGRFeature* _feature;
};

template <>
inline void Feature::set_field<time_point>(int /*index*/, const time_point& /*value*/)
{
    throw RuntimeError("Not implemented");
}

template <>
inline void Feature::set_field<date_point>(int /*index*/, const date_point& /*value*/)
{
    throw RuntimeError("Not implemented");
}

template <>
inline void Feature::set_field<int64_t>(int index, const int64_t& value)
{
    _feature->SetField(index, GIntBig(value));
}

template <>
inline void Feature::set_field<std::string_view>(int index, const std::string_view& value)
{
    _feature->SetField(index, std::string(value).c_str());
}

template <>
inline void Feature::set_field<std::string>(int index, const std::string& value)
{
    _feature->SetField(index, value.c_str());
}

template <>
inline void Feature::set_field<std::string_view>(const char* name, const std::string_view& value)
{
    _feature->SetField(name, std::string(value).c_str());
}

template <>
inline void Feature::set_field<std::string>(const char* name, const std::string& value)
{
    _feature->SetField(name, value.c_str());
}

template <>
inline void Feature::set_field<std::string_view>(const std::string& name, const std::string_view& value)
{
    _feature->SetField(name.c_str(), std::string(value).c_str());
}

template <>
inline void Feature::set_field<std::string>(const std::string& name, const std::string& value)
{
    _feature->SetField(name.c_str(), value.c_str());
}

struct IntersectionOptions
{
    std::string inputPrefix;                   // Set a prefix for the field names that will be created from the fields of the input layer.
    std::string methodPrefix;                  // Set a prefix for the field names that will be created from the fields of the method layer.
    bool skipFailures                 = false; // Set to true to go on, even when a feature could not be inserted or a GEOS call failed.
    bool promoteToMulti               = false; // Set to true to convert Polygons into MultiPolygons, or LineStrings to MultiLineStrings.
    bool usePreparedGeometries        = true;  // Set to false to not use prepared geometries to pretest intersection of features of method layer with features of this layer.
    bool preTestContainment           = false; // Set to true to pretest the containment of features of method layer within the features of this layer.This will speed up the method significantly in some cases.Requires that the prepared geometries are in effect.
    bool keepLowerDimensionGeometries = true;  // Set to false to skip result features with lower dimension geometry that would otherwise be added

    std::vector<std::string> additionalOptions;
    ProgressInfo progress;
};

struct ClipOptions
{
    std::string inputPrefix;     // Set a prefix for the field names that will be created from the fields of the input layer.
    std::string methodPrefix;    // Set a prefix for the field names that will be created from the fields of the method layer.
    bool skipFailures   = false; // Set to true to go on, even when a feature could not be inserted or a GEOS call failed.
    bool promoteToMulti = false; // Set to true to convert Polygons into MultiPolygons, or LineStrings to MultiLineStrings.

    std::vector<std::string> additionalOptions;
    ProgressInfo progress;
};

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

    Layer& operator=(Layer&&);
    Layer& operator=(const Layer&);

    std::optional<int32_t> epsg() const;
    //! Make sure the spatial reference stays in scope while using the layer!
    void set_projection(SpatialReference& srs);
    void set_projection_from_epsg(int32_t epsg);

    std::optional<SpatialReference> projection() const;

    void set_ignored_fields(const std::vector<std::string>& fieldnames);

    int64_t feature_count() const;
    /*! obtain the feature with the specified id
     * The id is not an index! Don't expect iteration from 0 to feature_count to work
     */
    Feature feature(int64_t id) const;

    int field_index(const char* name) const;
    int field_index(const std::string& name) const;
    int required_field_index(const char* name) const;
    int required_field_index(const std::string& name) const;
    void set_spatial_filter(Point<double> point);
    void set_spatial_filter(Point<double> topLeft, Point<double> bottomRight);
    void set_spatial_filter(GeometryRef geometry);
    void clear_spatial_filter();

    void set_attribute_filter(const char* name);
    void set_attribute_filter(const std::string& name);
    void clear_attribute_filter();

    void create_field(FieldDefinition& field);
    void create_feature(Feature& feature);
    void set_feature(Feature& feature);

    FeatureDefinition layer_definition() const;
    Geometry::Type geometry_type() const;

    const char* name() const;
    Rect<double> extent() const;

    OGRLayer* get();
    const OGRLayer* get() const;

    OGRLayerH handle();
    const void* handle() const;

    void intersection(Layer& method, Layer& output);
    void intersection(Layer& method, Layer& output, IntersectionOptions& options);

    void clip(Layer& method, Layer& output);
    void clip(Layer& method, Layer& output, ClipOptions& options);

    void set_metadata(const std::string& name, const std::string& value, const std::string& domain = "");
    std::string metadata_item(const std::string& name, const std::string& domain = "");
    std::unordered_map<std::string, std::string> metadata(const std::string& domain = "");

    bool test_capability(const char* name);

private:
    OGRLayer* _layer = nullptr;
};

// Iteration is not thread safe!
// Do not iterate simultaneously from different threads.
template <typename TLayer>
class LayerIterator
{
public:
    using difference_type   = ptrdiff_t;
    using value_type        = std::conditional_t<std::is_const_v<TLayer>, const Feature, Feature>;
    using reference         = value_type&;
    using pointer           = value_type*;
    using iterator_category = std::forward_iterator_tag;

    LayerIterator()
    : _layer(nullptr)
    , _currentFeature(nullptr)
    {
    }

    LayerIterator(Layer layer)
    : _layer(std::move(layer))
    , _currentFeature(nullptr)
    {
        _layer.get()->ResetReading();
        next();
    }

    reference operator*()
    {
        return _currentFeature;
    }

    pointer operator->()
    {
        return &_currentFeature;
    }

    LayerIterator& operator++()
    {
        next();
        return *this;
    }

    LayerIterator& operator=(LayerIterator&& other)
    {
        if (this != &other) {
            _layer          = std::move(other._layer);
            _currentFeature = std::move(other._currentFeature);
        }

        return *this;
    }

    bool operator==(const LayerIterator& other) const
    {
        return _currentFeature == other._currentFeature;
    }

    bool operator!=(const LayerIterator& other) const
    {
        return !(*this == other);
    }

private:
    void next()
    {
        _currentFeature = Feature(_layer.get()->GetNextFeature());
    }

    Layer _layer;
    Feature _currentFeature;
};

// support for range based for loops
inline LayerIterator<Layer> begin(Layer& layer)
{
    return LayerIterator<Layer>(layer);
}

inline LayerIterator<const Layer> begin(const Layer& layer)
{
    return LayerIterator<const Layer>(layer);
}

inline LayerIterator<Layer> begin(Layer&& layer)
{
    return LayerIterator<Layer>(layer);
}

inline LayerIterator<Layer> end(Layer& /*layer*/)
{
    return LayerIterator<Layer>();
}

inline LayerIterator<const Layer> end(const Layer& /*layer*/)
{
    return LayerIterator<const Layer>();
}

inline LayerIterator<Layer> end(Layer&& layer)
{
    return LayerIterator<Layer>(layer);
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
    FeatureDefinitionIterator(FeatureDefinition featureDef);
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

    FeatureDefinition _featureDef;
    int _fieldCount        = 0;
    int _currentFieldIndex = 0;
    FieldDefinitionRef _currentField;
};

// support for range based for loops
inline FeatureDefinitionIterator begin(FeatureDefinition featDef)
{
    return FeatureDefinitionIterator(featDef);
}

inline FeatureDefinitionIterator end(FeatureDefinition featDef)
{
    return FeatureDefinitionIterator(featDef.field_count());
}

MultiLineRef force_to_multiLine(GeometryRef geom);

namespace Geometry {
template <Type type>
struct OGRTypeResolve
{
    using geometry_type = OGRGeometry;
};

template <>
struct OGRTypeResolve<Type::Point>
{
    using geometry_type = OGRPoint;
};

template <>
struct OGRTypeResolve<Type::Collection>
{
    using geometry_type = OGRGeometryCollection;
};

template <>
struct OGRTypeResolve<Type::Line>
{
    using geometry_type = OGRLineString;
};

template <>
struct OGRTypeResolve<Type::MultiLine>
{
    using geometry_type = OGRMultiLineString;
};

template <>
struct OGRTypeResolve<Type::LinearRing>
{
    using geometry_type = OGRLinearRing;
};

template <>
struct OGRTypeResolve<Type::Polygon>
{
    using geometry_type = OGRPolygon;
};

template <>
struct OGRTypeResolve<Type::MultiPolygon>
{
    using geometry_type = OGRMultiPolygon;
};

template <Type type>
struct WrapperTypeResolve
{
    using wrapper_type = GeometryRef;
};

template <>
struct WrapperTypeResolve<Type::Point>
{
    using wrapper_type = PointRef;
};

template <>
struct WrapperTypeResolve<Type::Collection>
{
    using wrapper_type = GeometryCollectionRef;
};

template <>
struct WrapperTypeResolve<Type::Line>
{
    using wrapper_type = LineRef;
};

template <>
struct WrapperTypeResolve<Type::MultiLine>
{
    using wrapper_type = MultiLineRef;
};

template <>
struct WrapperTypeResolve<Type::LinearRing>
{
    using wrapper_type = LinearRingRef;
};

template <>
struct WrapperTypeResolve<Type::Polygon>
{
    using wrapper_type = PolygonRef;
};

template <>
struct WrapperTypeResolve<Type::MultiPolygon>
{
    using wrapper_type = MultiPolygonRef;
};

template <Type type>
using ogr_geometry_type_t = typename OGRTypeResolve<type>::geometry_type;

template <Type type>
using wrapped_type_t = typename WrapperTypeResolve<type>::wrapper_type;

template <Type type>
Owner<wrapped_type_t<type>> create()
{
    return Owner<wrapped_type_t<type>>(new ogr_geometry_type_t<type>());
}
}
}
