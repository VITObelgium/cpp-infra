#include "gdx/raster.h"
#include "gdx/algo/cast.h"
#include "gdx/maskedrasterio.h"
#include "gdx/rastercompare.h"
#include "gdx/rasterio.h"

namespace gdx {

namespace {

Raster::RasterVariant createRasterVariant(RasterMetadata meta, const std::type_info& dataType)
{
    if (dataType == typeid(uint8_t))
        return Raster::RasterVariant(MaskedRaster<uint8_t>(std::move(meta)));
    else if (dataType == typeid(int16_t))
        return Raster::RasterVariant(MaskedRaster<int16_t>(std::move(meta)));
    else if (dataType == typeid(uint16_t))
        return Raster::RasterVariant(MaskedRaster<uint16_t>(std::move(meta)));
    else if (dataType == typeid(int32_t))
        return Raster::RasterVariant(MaskedRaster<int32_t>(std::move(meta)));
    else if (dataType == typeid(uint32_t))
        return Raster::RasterVariant(MaskedRaster<uint32_t>(std::move(meta)));
    else if (dataType == typeid(float))
        return Raster::RasterVariant(MaskedRaster<float>(std::move(meta)));
    else if (dataType == typeid(double))
        return Raster::RasterVariant(MaskedRaster<double>(std::move(meta)));

    throw InvalidArgument("Invalid raster data type");
}

Raster::RasterVariant createRasterVariant(RasterMetadata meta, const std::type_info& dataType, double fillValue)
{
    if (dataType == typeid(uint8_t))
        return Raster::RasterVariant(MaskedRaster<uint8_t>(std::move(meta), static_cast<uint8_t>(fillValue)));
    else if (dataType == typeid(int16_t))
        return Raster::RasterVariant(MaskedRaster<int16_t>(std::move(meta), static_cast<int16_t>(fillValue)));
    else if (dataType == typeid(uint16_t))
        return Raster::RasterVariant(MaskedRaster<uint16_t>(std::move(meta), static_cast<uint16_t>(fillValue)));
    else if (dataType == typeid(int32_t))
        return Raster::RasterVariant(MaskedRaster<int32_t>(std::move(meta), static_cast<int32_t>(fillValue)));
    else if (dataType == typeid(uint32_t))
        return Raster::RasterVariant(MaskedRaster<uint32_t>(std::move(meta), static_cast<uint32_t>(fillValue)));
    else if (dataType == typeid(float))
        return Raster::RasterVariant(MaskedRaster<float>(std::move(meta), static_cast<float>(fillValue)));
    else if (dataType == typeid(double))
        return Raster::RasterVariant(MaskedRaster<double>(std::move(meta), fillValue));

    throw InvalidArgument("Invalid raster data type");
}

template <typename T>
using value_type = typename std::remove_cv_t<std::remove_reference_t<T>>::value_type;

template <typename TScalar>
void throwOnScalarTypeMismatch(const Raster::RasterVariant& var)
{
    // It is allowed to e.g. add integers to floats, but it is not allowed to add floats to integers
    std::visit([](auto&& raster) {
        using T = value_type<decltype(raster)>;
        if constexpr (!std::is_floating_point_v<T> && std::is_floating_point_v<TScalar>) {
            throw InvalidArgument("Mixing of floating point and integer values is not supported Raster type ({}) <-> Scalar type ({})", typeid(T).name(), typeid(TScalar).name());
        }
    },
        var);
}
}

Raster::Raster(int32_t rows, int32_t cols, const std::type_info& dataType)
: Raster(RasterMetadata(rows, cols), dataType)
{
}

Raster::Raster(int32_t rows, int32_t cols, const std::type_info& dataType, double fillValue)
: Raster(RasterMetadata(rows, cols), dataType, fillValue)
{
}

Raster::RasterVariant& Raster::get()
{
    return _raster;
}

const Raster::RasterVariant& Raster::get() const
{
    return _raster;
}

Raster::Raster(RasterMetadata meta, const std::type_info& dataType)
: _type(dataType)
, _raster(createRasterVariant(std::move(meta), dataType))
{
}

Raster::Raster(RasterMetadata meta, const std::type_info& dataType, double fillValue)
: _type(dataType)
, _raster(createRasterVariant(std::move(meta), dataType, fillValue))
{
}

const std::type_info& Raster::type() const noexcept
{
    return _type;
}

const RasterMetadata& Raster::metadata() const
{
    return std::visit([](auto&& raster) {
        return std::cref(raster.metadata());
    },
        _raster);
}

Raster Raster::copy() const
{
    return std::visit([](auto&& raster) {
        return Raster(raster.copy());
    },
        _raster);
}

bool Raster::equalTo(const Raster& other) const noexcept
{
    if (type() != other.type()) {
        return false;
    }

    return std::visit([&other](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return raster == other.get<T>();
    },
        _raster);
}

bool Raster::tolerant_equal_to(const Raster& other, double tolerance) const noexcept
{
    if (type() != other.type()) {
        return false;
    }

    return std::visit([&other, tolerance](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return raster.tolerant_equal_to(other.get<T>(), static_cast<T>(tolerance));
    },
        _raster);
}

bool Raster::tolerant_data_equal_to(const Raster& other, double tolerance) const noexcept
{
    if (type() != other.type()) {
        return false;
    }

    return std::visit([&other, tolerance](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return raster.tolerant_data_equal_to(other.get<T>(), static_cast<T>(tolerance));
    },
        _raster);
}

Raster& Raster::fill(double value)
{
    std::visit([value](auto&& raster) {
        using T = value_type<decltype(raster)>;
        raster.fill(static_cast<T>(value));
    },
        _raster);

    return *this;
}

Raster& Raster::set_projection(int32_t epsg)
{
    std::visit([epsg](auto&& raster) {
        raster.set_projection(epsg);
    },
        _raster);

    return *this;
}

Raster& Raster::replaceValue(double oldValue, double newValue)
{
    std::visit([oldValue, newValue](auto&& raster) {
        using T = value_type<decltype(raster)>;
        raster.replace(static_cast<T>(oldValue), static_cast<T>(newValue));
    },
        _raster);

    return *this;
}

void Raster::setValue(int32_t row, int32_t col, double value)
{
    std::visit([row, col, value](auto&& raster) {
        if (row < 0 || row >= raster.rows() ||
            col < 0 || col >= raster.cols()) {
            throw InvalidArgument("Invalid row column {},{}", row, col);
        }
        using T          = value_type<decltype(raster)>;
        raster(row, col) = static_cast<T>(value);
        raster.mark_as_data(row, col);
    },
        _raster);
}

Raster::operator bool() const
{
    return std::visit([](auto&& raster) {
        return raster.size() > 0;
    },
        _raster);
}

Raster Raster::operator+(int32_t value) const
{
    throwOnScalarTypeMismatch<decltype(value)>();
    return std::visit([value](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster + static_cast<T>(value));
    },
        _raster);
}

Raster Raster::operator+(double value) const
{
    throwOnScalarTypeMismatch<decltype(value)>();
    return std::visit([value](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster + static_cast<T>(value));
    },
        _raster);
}

Raster Raster::operator-() const
{
    return std::visit([](auto&& raster) {
        return Raster(-raster);
    },
        _raster);
}

Raster Raster::operator-(int32_t value) const
{
    throwOnScalarTypeMismatch<decltype(value)>();
    return std::visit([value](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster - static_cast<T>(value));
    },
        _raster);
}

Raster Raster::operator-(double value) const
{
    throwOnScalarTypeMismatch<decltype(value)>();
    return std::visit([value](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster - static_cast<T>(value));
    },
        _raster);
}

Raster Raster::operator*(int32_t value) const
{
    throwOnScalarTypeMismatch<decltype(value)>();
    return std::visit([value](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster * static_cast<T>(value));
    },
        _raster);
}

Raster Raster::operator*(double value) const
{
    throwOnScalarTypeMismatch<decltype(value)>();
    return std::visit([value](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster * static_cast<T>(value));
    },
        _raster);
}

Raster Raster::operator/(int32_t value) const
{
    throwOnScalarTypeMismatch<decltype(value)>();
    return std::visit([value](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster / static_cast<T>(value));
    },
        _raster);
}

Raster Raster::operator/(double value) const
{
    throwOnScalarTypeMismatch<decltype(value)>();
    return std::visit([value](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster / static_cast<T>(value));
    },
        _raster);
}

Raster Raster::operator+(const Raster& other) const
{
    return std::visit([](auto&& raster1, auto&& raster2) {
        return Raster(raster1 + raster2);
    },
        _raster, other._raster);
}

Raster Raster::operator-(const Raster& other) const
{
    return std::visit([](auto&& raster1, auto&& raster2) {
        return Raster(raster1 - raster2);
    },
        _raster, other._raster);
}

Raster Raster::operator*(const Raster& other) const
{
    return std::visit([](auto&& raster1, auto&& raster2) {
        return Raster(raster1 * raster2);
    },
        _raster, other._raster);
}

Raster Raster::operator/(const Raster& other) const
{
    return std::visit([](auto&& raster1, auto&& raster2) {
        return Raster(raster1 / raster2);
    },
        _raster, other._raster);
}

Raster operator+(int32_t lhs, const Raster& rhs)
{
    return rhs + lhs;
}

Raster operator+(double lhs, const Raster& rhs)
{
    return rhs + lhs;
}

Raster operator-(int32_t lhs, const Raster& rhs)
{
    throwOnScalarTypeMismatch<decltype(lhs)>(rhs.get());

    return std::visit([lhs](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(static_cast<T>(lhs) - raster);
    },
        rhs.get());
}

Raster operator-(double lhs, const Raster& rhs)
{
    throwOnScalarTypeMismatch<decltype(lhs)>(rhs.get());

    return std::visit([lhs](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(static_cast<T>(lhs) - raster);
    },
        rhs.get());
}

Raster operator*(int32_t lhs, const Raster& rhs)
{
    return rhs * lhs;
}

Raster operator*(double lhs, const Raster& rhs)
{
    return rhs * lhs;
}

Raster operator/(int32_t lhs, const Raster& rhs)
{
    throwOnScalarTypeMismatch<decltype(lhs)>(rhs.get());

    return std::visit([lhs](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(static_cast<T>(lhs) / raster);
    },
        rhs.get());
}

Raster operator/(double lhs, const Raster& rhs)
{
    throwOnScalarTypeMismatch<decltype(lhs)>(rhs.get());

    return std::visit([lhs](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(static_cast<T>(lhs) / raster);
    },
        rhs.get());
}

Raster Raster::operator!() const
{
    return std::visit([](auto&& raster) {
        return Raster(!raster);
    },
        _raster);
}

Raster Raster::operator&&(const Raster& other) const
{
    return std::visit([](auto&& raster1, auto&& raster2) {
        return Raster(raster1 && raster2);
    },
        _raster, other._raster);
}

Raster Raster::operator||(const Raster& other) const
{
    return std::visit([](auto&& raster1, auto&& raster2) {
        return Raster(raster1 || raster2);
    },
        _raster, other._raster);
}

Raster Raster::operator>(const Raster& other) const
{
    return std::visit([&other](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster > other.get<T>());
    },
        _raster);
}

Raster Raster::operator>(double threshold) const
{
    return std::visit([threshold](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster > static_cast<T>(threshold));
    },
        _raster);
}

Raster Raster::operator>=(const Raster& other) const
{
    return std::visit([&other](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster >= other.get<T>());
    },
        _raster);
}

Raster Raster::operator>=(double threshold) const
{
    return std::visit([threshold](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster >= static_cast<T>(threshold));
    },
        _raster);
}

Raster Raster::operator<(const Raster& other) const
{
    return std::visit([&other](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster < other.get<T>());
    },
        _raster);
}

Raster Raster::operator<(double threshold) const
{
    return std::visit([threshold](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster < static_cast<T>(threshold));
    },
        _raster);
}

Raster Raster::operator<=(const Raster& other) const
{
    return std::visit([&other](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster <= other.get<T>());
    },
        _raster);
}

Raster Raster::operator<=(double threshold) const
{
    return std::visit([threshold](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster <= static_cast<T>(threshold));
    },
        _raster);
}

Raster Raster::operator==(const Raster& other) const
{
    return std::visit([](auto&& raster1, auto&& raster2) {
        return Raster(equals(raster1, raster2));
    },
        _raster, other.get());
}

Raster Raster::operator==(int32_t value) const
{
    throwOnScalarTypeMismatch<decltype(value)>();

    return std::visit([value](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(equals(raster, static_cast<T>(value)));
    },
        _raster);
}

Raster Raster::operator==(double value) const
{
    throwOnScalarTypeMismatch<decltype(value)>();

    return std::visit([value](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(equals(raster, static_cast<T>(value)));
    },
        _raster);
}

Raster Raster::operator!=(const Raster& other) const
{
    throwOnTypeMismatch(other.type());

    return std::visit([&other](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster.not_equals(other.get<T>()));
    },
        _raster);
    return Raster(MaskedRaster<uint8_t>());
}

Raster Raster::operator!=(int32_t value) const
{
    throwOnScalarTypeMismatch<decltype(value)>();

    return std::visit([value](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster.not_equals(static_cast<T>(value)));
    },
        _raster);
}

Raster Raster::operator!=(double value) const
{
    throwOnScalarTypeMismatch<decltype(value)>();

    return std::visit([value](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(raster.not_equals(static_cast<T>(value)));
    },
        _raster);
}

Raster Raster::cast(const std::type_info& type) const
{
    return std::visit([&type](auto&& raster) {
        if (type == typeid(uint8_t)) return Raster(raster_cast<uint8_t>(raster));
        if (type == typeid(int16_t)) return Raster(raster_cast<int16_t>(raster));
        if (type == typeid(uint16_t)) return Raster(raster_cast<uint16_t>(raster));
        if (type == typeid(int32_t)) return Raster(raster_cast<int32_t>(raster));
        if (type == typeid(uint32_t)) return Raster(raster_cast<uint32_t>(raster));
        if (type == typeid(float)) return Raster(raster_cast<float>(raster));
        if (type == typeid(double)) return Raster(raster_cast<double>(raster));

        throw RuntimeError("Unsupported raster data type");
    },
        _raster);
}

void Raster::throwOnTypeMismatch(const std::type_info& other) const
{
    if (other != _type) {
        throw InvalidArgument("Raster types are different {} <-> {}", _type.name(), other.name());
    }
}

template <typename TOtherRaster>
void Raster::throwOnTypeMismatch() const
{
    std::visit([](auto&& raster) {
        using T = value_type<decltype(raster)>;
        if (std::is_floating_point_v<T> != std::is_floating_point_v<TOtherRaster>) {
            throw InvalidArgument("Mixing of floating point and integer rasters is not supported {} <-> {}", typeid(T).name(), typeid(TOtherRaster).name());
        }
    },
        _raster);
}

template <typename TScalar>
void Raster::throwOnScalarTypeMismatch() const
{
    gdx::throwOnScalarTypeMismatch<TScalar>(_raster);
}

Raster Raster::read(const fs::path& fileName)
{
    return read(fileName, io::get_raster_type(fileName));
}

Raster Raster::read(const fs::path& fileName, const RasterMetadata& extent)
{
    return read(fileName, io::get_raster_type(fileName), extent);
}

Raster Raster::read(const fs::path& fileName, const std::type_info& type)
{
    if (type == typeid(uint8_t)) return read_masked_raster<uint8_t>(fileName);
    if (type == typeid(int16_t)) return read_masked_raster<int16_t>(fileName);
    if (type == typeid(uint16_t)) return read_masked_raster<uint16_t>(fileName);
    if (type == typeid(int32_t)) return read_masked_raster<int32_t>(fileName);
    if (type == typeid(uint32_t)) return read_masked_raster<uint32_t>(fileName);
    if (type == typeid(float)) return read_masked_raster<float>(fileName);
    if (type == typeid(double)) return read_masked_raster<double>(fileName);

    throw RuntimeError("Unsupported raster data type");
}

Raster Raster::read(const fs::path& fileName, const std::type_info& type, const RasterMetadata& extent)
{
    if (type == typeid(uint8_t)) return read_masked_raster<uint8_t>(fileName, extent);
    if (type == typeid(int16_t)) return read_masked_raster<int16_t>(fileName, extent);
    if (type == typeid(uint16_t)) return read_masked_raster<uint16_t>(fileName, extent);
    if (type == typeid(int32_t)) return read_masked_raster<int32_t>(fileName, extent);
    if (type == typeid(uint32_t)) return read_masked_raster<uint32_t>(fileName, extent);
    if (type == typeid(float)) return read_masked_raster<float>(fileName, extent);
    if (type == typeid(double)) return read_masked_raster<double>(fileName, extent);

    throw RuntimeError("Unsupported raster data type");
}

Raster Raster::readFromMemory(const fs::path& path, gsl::span<const uint8_t> data)
{
    io::gdal::MemoryFile memFile(path.u8string(), data);
    return read(path);
}

Raster Raster::readFromMemory(const fs::path& path, gsl::span<const uint8_t> data, const std::type_info& type)
{
    io::gdal::MemoryFile memFile(path.u8string(), data);
    return read(path, type);
}

// Writes the raster to disk using the type of the raster
void Raster::write(const fs::path& filepath)
{
    std::visit([&filepath](auto&& raster) {
        gdx::write_raster(raster, filepath);
    },
        _raster);
}

// Writes the raster to disk using the provided type
void Raster::write(const fs::path& filepath, const std::type_info& type)
{
    std::visit([&filepath, &type](auto&& raster) {
        gdx::write_raster(raster, filepath, type);
    },
        _raster);
}

void Raster::writeColorMapped(const fs::path& filepath, const inf::ColorMap& cmap) const
{
    std::visit([&filepath, &cmap](auto&& raster) {
        gdx::io::write_raster_color_mapped(raster, raster.metadata(), filepath, cmap);
    },
        _raster);
}

double Raster::nodataForType(const std::type_info& dataType)
{
    if (dataType == typeid(uint8_t))
        return std::numeric_limits<uint8_t>::max();
    else if (dataType == typeid(int16_t))
        return std::numeric_limits<int16_t>::max();
    else if (dataType == typeid(uint16_t))
        return std::numeric_limits<uint16_t>::max();
    else if (dataType == typeid(int32_t))
        return std::numeric_limits<int32_t>::max();
    else if (dataType == typeid(uint32_t))
        return std::numeric_limits<uint32_t>::max();
    else if (dataType == typeid(float))
        return std::numeric_limits<float>::max();
    else if (dataType == typeid(double))
        return std::numeric_limits<double>::max();

    throw InvalidArgument("Invalid raster data type");
}

// void Raster::writeColorMapped(const std::string& filepath, const std::type_info& type, const ColorMap& cmap) const
// {
//     std::visit([this, &filepath, &type, &cmap] (auto&& raster) {
//         gdx::io::write_rasterColorMapped(raster, filepath, cmap);
//     }, _raster);
// }
}
