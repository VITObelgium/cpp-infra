#include "infra/gdalgeometry.h"
#include "infra/cast.h"
#include "infra/conversion.h"
#include "infra/exception.h"
#include "infra/gdal-private.h"
#include "infra/gdal.h"
#include "infra/string.h"

namespace inf::gdal {

using namespace std::string_literals;

Geometry::Type geometry_type_from_gdal(OGRwkbGeometryType type)
{
    switch (wkbFlatten(type)) {
    case wkbUnknown:
        return Geometry::Type::Unknown;
    case wkbPoint:
        return Geometry::Type::Point;
    case wkbMultiPoint:
        return Geometry::Type::MultiPoint;
    case wkbLineString:
        return Geometry::Type::Line;
    case wkbPolygon:
        return Geometry::Type::Polygon;
    case wkbCurvePolygon:
        return Geometry::Type::CurvePolygon;
    case wkbMultiPolygon:
        return Geometry::Type::MultiPolygon;
    case wkbMultiLineString:
        return Geometry::Type::MultiLine;
    case wkbGeometryCollection:
        return Geometry::Type::GeometryCollection;
    case wkbMultiSurface:
        return Geometry::Type::MultiSurface;
    case wkbMultiCurve:
        return Geometry::Type::MultiCurve;
    case wkbNone:
        return Geometry::Type::None;
    default:
        throw RuntimeError("Unsupported geometry type ({})", static_cast<std::underlying_type_t<OGRwkbGeometryType>>(wkbFlatten(type)));
    }
}

Envelope::Envelope(double minX, double maxX, double minY, double maxY) noexcept
{
    _envelope.MinX = minX;
    _envelope.MinY = minY;
    _envelope.MaxX = maxX;
    _envelope.MaxY = maxY;
}

Envelope::Envelope(Point<int64_t> topleft, Point<int64_t> bottomRight) noexcept
: Envelope(double(topleft.x), double(bottomRight.x), double(bottomRight.y), double(topleft.y))
{
}

Envelope::Envelope(Point<double> topleft, Point<double> bottomRight) noexcept
: Envelope(topleft.x, bottomRight.x, bottomRight.y, topleft.y)
{
}

OGREnvelope* Envelope::get() noexcept
{
    return &_envelope;
}

const OGREnvelope* Envelope::get() const noexcept
{
    return &_envelope;
}

Envelope::operator bool() const noexcept
{
    return _envelope.IsInit() != 0;
}

void Envelope::merge(const Envelope& other) noexcept
{
    _envelope.Merge(*other.get());
}

void Envelope::merge(double x, double y) noexcept
{
    _envelope.Merge(x, y);
}

void Envelope::intersect(const Envelope& other) noexcept
{
    _envelope.Intersect(*other.get());
}

bool Envelope::intersects(const Envelope& other) const noexcept
{
    return _envelope.Intersects(*other.get()) != 0;
}

bool Envelope::contains(const Envelope& other) const noexcept
{
    return _envelope.Contains(*other.get()) != 0;
}

Point<double> Envelope::top_left() const noexcept
{
    return Point<double>(_envelope.MinX, _envelope.MaxY);
}

Point<double> Envelope::bottom_right() const noexcept
{
    return Point<double>(_envelope.MaxX, _envelope.MinY);
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

FeatureDefinition::FeatureDefinition(const FeatureDefinition& other)
: _def(other._def)
{
    if (_def) {
        _def->Reference();
    }
}

FeatureDefinition::FeatureDefinition(const char* name)
: _def(new OGRFeatureDefn(name))
{
    _def->Reference();
}

FeatureDefinition::FeatureDefinition(OGRFeatureDefn* def)
: _def(def)
{
    assert(def);
    _def->Reference();
}

FeatureDefinition::~FeatureDefinition()
{
    if (_def) {
        _def->Release();
    }
}

std::string_view FeatureDefinition::name() const
{
    return std::string_view(_def->GetName());
}

int FeatureDefinition::field_count() const
{
    return _def->GetFieldCount();
}

int FeatureDefinition::field_index(const char* name) const noexcept
{
    return _def->GetFieldIndex(name);
}

int FeatureDefinition::field_index(const std::string& name) const noexcept
{
    return field_index(name.c_str());
}

int FeatureDefinition::required_field_index(const char* name) const
{
    int index = _def->GetFieldIndex(name);
    if (index < 0) {
        throw RuntimeError("Field not present: {}", name);
    }

    return index;
}

int FeatureDefinition::required_field_index(const std::string& name) const
{
    return required_field_index(name.c_str());
}

FieldDefinitionRef FeatureDefinition::field_definition(int index) const
{
    return FieldDefinitionRef(check_pointer(_def->GetFieldDefn(index), "Failed to obtain field definition"));
}

OGRFeatureDefn* FeatureDefinition::get() noexcept
{
    return _def;
}

Feature::Feature(FeatureDefinition featurDef)
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

GeometryRef Feature::geometry()
{
    return GeometryRef(check_pointer(_feature->GetGeometryRef(), "No geometry present"));
}

GeometryCRef Feature::geometry() const
{
    return const_cast<Feature*>(this)->geometry();
}

bool Feature::has_geometry() const noexcept
{
    return _feature->GetGeometryRef() != nullptr;
}

int64_t Feature::id() const
{
    return _feature->GetFID();
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

bool Feature::field_is_valid(int index) const noexcept
{
    return _feature->IsFieldSetAndNotNull(index);
}

FieldDefinitionRef Feature::field_definition(int index) const
{
    return FieldDefinitionRef(check_pointer(_feature->GetFieldDefnRef(index), "Invalid field definition index"));
}

FeatureDefinition Feature::feature_definition() const
{
    return FeatureDefinition(check_pointer(_feature->GetDefnRef(), "Failed to obtain feature definition"));
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

std::optional<Field> Feature::opt_field(int index) const noexcept
{
    assert(index < field_count());
    if (!field_is_valid(index)) {
        return {};
    }

    return field(index);
}

template <typename T>
T Feature::field_as(int index) const
{
#if __cplusplus > 201703L
    namespace date = std::chrono;
#endif

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
        assert(_feature->GetDefnRef()->GetFieldDefn(index)->GetType() == OFTString);
        return std::string_view(_feature->GetFieldAsString(index));
    } else if constexpr (std::is_same_v<time_point, T>) {
        int year, month, day, hour, minute, timezoneFlag;
        float second;
        if (_feature->GetFieldAsDateTime(index, &year, &month, &day, &hour, &minute, &second, &timezoneFlag) == FALSE) {
            throw RuntimeError("Failed to get field as time point {}", _feature->GetFieldAsString(index));
        }

        auto date      = date::year_month_day(date::year(year), date::month(month), date::day(day));
        auto timePoint = std::chrono::time_point_cast<std::chrono::milliseconds>(static_cast<gdal::sys_days>(date));
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

    T field = field_as<T>(index);
    if constexpr (std::is_scalar_v<T>) {
        if (field == 0 && field_as<std::string>(index).empty()) {
            // it was en empty string that converts to 0 and we want it to be an empty optional
            return {};
        }
    }

    return field;
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

void Feature::set_field_to_null(int index)
{
    _feature->SetFieldNull(index);
}

void Feature::set_field(int index, const Field& field)
{
    std::visit([this, index](const auto& val) {
        set_field<std::decay_t<decltype(val)>>(index, val);
    },
               field);
}

void Feature::set_from(const Feature& other, FieldCopyMode mode)
{
    check_error(_feature->SetFrom(other.get(), mode == FieldCopyMode::Forgiving ? TRUE : FALSE), "Failed to copy feature geometry and fields");
}

void Feature::set_from(const Feature& other, FieldCopyMode mode, std::span<int32_t> fieldIndexes)
{
    assert(truncate<int>(fieldIndexes.size()) == other.field_count());
    check_error(_feature->SetFrom(other.get(), fieldIndexes.data(), mode == FieldCopyMode::Forgiving ? TRUE : FALSE), "Failed to copy feature geometry and fields");
}

void Feature::set_fields_from(const Feature& other, FieldCopyMode mode, std::span<int32_t> fieldIndexes)
{
    assert(truncate<int>(fieldIndexes.size()) == other.field_count());
    check_error(_feature->SetFieldsFrom(other.get(), fieldIndexes.data(), mode == FieldCopyMode::Forgiving ? TRUE : FALSE), "Failed to copy feature fields");
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

Layer& Layer::operator=(Layer&& other)
{
    _layer       = other._layer;
    other._layer = nullptr;
    return *this;
}

Layer& Layer::operator=(const Layer& other)
{
    _layer = other._layer;
    _layer->Reference();
    return *this;
}

std::optional<int32_t> Layer::epsg() const
{
    if (auto* spatialRef = _layer->GetSpatialRef(); spatialRef != nullptr) {
        if (auto* epsg = _layer->GetSpatialRef()->GetAuthorityCode("PROJCS"); epsg != nullptr) {
            return str::to_int32(epsg);
        }

        if (auto* epsg = spatialRef->GetAttrValue("PROJCS|GEOGCS|AUTHORITY", 1); epsg != nullptr) {
            return str::to_int32(epsg);
        }
    }

    return std::optional<int32_t>();
}

void Layer::set_projection(SpatialReference& srs)
{
    const int geomCount = _layer->GetLayerDefn()->GetGeomFieldCount();
    for (int i = 0; i < geomCount; ++i) {
        _layer->GetLayerDefn()->GetGeomFieldDefn(i)->SetSpatialRef(srs.get());
    }
}

void Layer::set_projection_from_epsg(int32_t epsg)
{
    SpatialReference srs(epsg);
    const int geomCount = _layer->GetLayerDefn()->GetGeomFieldCount();
    for (int i = 0; i < geomCount; ++i) {
        _layer->GetLayerDefn()->GetGeomFieldDefn(i)->SetSpatialRef(srs.get());
    }
}

std::optional<SpatialReference> Layer::projection() const
{
    if (auto* srs = _layer->GetSpatialRef(); srs != nullptr) {
        return SpatialReference(srs);
    }

    return {};
}

void Layer::set_ignored_fields(const std::vector<std::string>& fieldNames)
{
    const auto fields = create_string_list(fieldNames);
    check_error(_layer->SetIgnoredFields(const_cast<const char**>(fields.List())), "Failed to ignore layer fields");
}

int64_t Layer::feature_count() const
{
    return _layer->GetFeatureCount();
}

Feature Layer::feature(int64_t index) const
{
    assert(index != OGRNullFID);
    return Feature(check_pointer(_layer->GetFeature(index), "Failed to get feature from layer"));
}

int Layer::field_index(const char* name) const
{
    return _layer->FindFieldIndex(name, 1 /*exact match*/);
}

int Layer::field_index(const std::string& name) const
{
    return field_index(name.c_str());
}

int Layer::required_field_index(const char* name) const
{
    if (auto index = _layer->FindFieldIndex(name, 1 /*exact match*/); index >= 0) {
        return index;
    }

    throw RuntimeError("Layer does not contain field with name '{}'", name);
}

int Layer::required_field_index(const std::string& name) const
{
    return required_field_index(name.c_str());
}

void Layer::set_spatial_filter(Point<double> point)
{
    OGRPoint p(point.x, point.y);
    _layer->SetSpatialFilter(&p);
}

void Layer::set_spatial_filter(Point<double> topLeft, Point<double> bottomRight)
{
    _layer->SetSpatialFilterRect(topLeft.x, bottomRight.y, bottomRight.x, topLeft.y);
}

void Layer::set_spatial_filter(GeometryRef geometry)
{
    _layer->SetSpatialFilter(geometry.get());
}

void Layer::clear_spatial_filter()
{
    _layer->SetSpatialFilter(nullptr);
}

void Layer::set_attribute_filter(const char* name)
{
    check_error(_layer->SetAttributeFilter(name), "Failed to set attribute filter");
}

void Layer::set_attribute_filter(const std::string& name)
{
    set_attribute_filter(name.c_str());
}

void Layer::clear_attribute_filter()
{
    set_attribute_filter(nullptr);
}

void Layer::create_field(FieldDefinition& field)
{
    check_error(_layer->CreateField(field.get()), "Failed to create layer field");
}

void Layer::create_feature(Feature& feature)
{
    check_error(_layer->CreateFeature(feature.get()), "Failed to create layer feature");
}

void Layer::set_feature(Feature& feature)
{
    check_error(_layer->SetFeature(feature.get()), "Failed to assign layer feature");
}

FeatureDefinition Layer::layer_definition() const
{
    return FeatureDefinition(check_pointer(_layer->GetLayerDefn(), "Failed to obtain layer definition"));
}

Geometry::Type Layer::geometry_type() const
{
    return geometry_type_from_gdal(_layer->GetGeomType());
}

const char* Layer::name() const
{
    return _layer->GetName();
}

Rect<double> Layer::extent() const
{
    OGREnvelope env;
    check_error(_layer->GetExtent(&env, TRUE), "Failed to get layer extent");

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

void Layer::intersection(Layer& method, Layer& output)
{
    check_error(_layer->Intersection(method.get(), output.get(), nullptr, nullptr, nullptr), "Failed to get layer intersection");
}

void Layer::intersection(Layer& method, Layer& output, IntersectionOptions& options)
{
    std::vector<std::string> optionsArray;

    GDALProgressFunc progressFunc = nullptr;
    void* progressArg             = nullptr;
    if (options.progress.is_valid()) {
        progressArg  = &options.progress;
        progressFunc = [](double complete, const char* /*message*/, void* progressArg) -> int {
            auto* cb = reinterpret_cast<ProgressInfo*>(progressArg);
            cb->tick(truncate<float>(complete));
            return cb->cancel_requested() ? FALSE : TRUE;
        };
    }

    constexpr size_t optionCount = 7;
    optionsArray.reserve(optionCount + options.additionalOptions.size());
    optionsArray.push_back(fmt::format("SKIP_FAILURES={}", options.skipFailures ? "YES" : "NO"));
    optionsArray.push_back(fmt::format("PROMOTE_TO_MULTI={}", options.promoteToMulti ? "YES" : "NO"));
    optionsArray.push_back(fmt::format("INPUT_PREFIX={}", options.inputPrefix));
    optionsArray.push_back(fmt::format("METHOD_PREFIX={}", options.methodPrefix));
    optionsArray.push_back(fmt::format("USE_PREPARED_GEOMETRIES={}", options.usePreparedGeometries ? "YES" : "NO"));
    optionsArray.push_back(fmt::format("PRETEST_CONTAINMENT={}", options.preTestContainment ? "YES" : "NO"));
    optionsArray.push_back(fmt::format("KEEP_LOWER_DIMENSION_GEOMETRIES={}", options.keepLowerDimensionGeometries ? "YES" : "NO"));
    for (auto& opt : options.additionalOptions) {
        optionsArray.push_back(opt);
    }

    auto opts = create_string_list(optionsArray);
    check_error(_layer->Intersection(method.get(), output.get(), opts.List(), progressFunc, progressArg), "Failed to get layer intersection");
}

void Layer::clip(Layer& method, Layer& output)
{
    check_error(_layer->Clip(method.get(), output.get(), nullptr, nullptr, nullptr), "Failed to clip layer");
}

void Layer::clip(Layer& method, Layer& output, ClipOptions& options)
{
    std::vector<std::string> optionsArray;

    GDALProgressFunc progressFunc = nullptr;
    void* progressArg             = nullptr;
    if (options.progress.is_valid()) {
        progressArg  = &options.progress;
        progressFunc = [](double complete, const char* /*message*/, void* progressArg) -> int {
            auto* cb = reinterpret_cast<ProgressInfo*>(progressArg);
            cb->tick(truncate<float>(complete));
            return cb->cancel_requested() ? FALSE : TRUE;
        };
    }

    constexpr size_t optionCount = 4;
    optionsArray.reserve(optionCount + options.additionalOptions.size());
    optionsArray.push_back(fmt::format("SKIP_FAILURES={}", options.skipFailures ? "YES" : "NO"));
    optionsArray.push_back(fmt::format("PROMOTE_TO_MULTI={}", options.promoteToMulti ? "YES" : "NO"));
    optionsArray.push_back(fmt::format("INPUT_PREFIX={}", options.inputPrefix));
    optionsArray.push_back(fmt::format("METHOD_PREFIX={}", options.methodPrefix));
    for (auto& opt : options.additionalOptions) {
        optionsArray.push_back(opt);
    }

    auto opts = create_string_list(optionsArray);
    check_error(_layer->Clip(method.get(), output.get(), opts.List(), progressFunc, progressArg), "Failed to clip layer");
}

void Layer::set_metadata(const std::string& name, const std::string& value, const std::string& domain)
{
    check_error(_layer->SetMetadataItem(name.c_str(), value.c_str(), domain.c_str()), "Failed to set layer metadata");
}

std::string Layer::metadata_item(const std::string& name, const std::string& domain)
{
    return check_pointer(_layer->GetMetadataItem(name.c_str(), domain.c_str()), "Failed to get layer metadata");
}

std::unordered_map<std::string, std::string> Layer::metadata(const std::string& domain)
{
    std::unordered_map<std::string, std::string> result;

    char** data = _layer->GetMetadata(domain.c_str());
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

bool Layer::test_capability(const char* name)
{
    return _layer->TestCapability(name) == TRUE;
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

FeatureDefinitionIterator::FeatureDefinitionIterator(FeatureDefinition featureDef)
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

}
