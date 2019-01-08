#pragma once

#include "gdx/cell.h"
#include "gdx/cpupredicates-private.h"
#include "gdx/eigeniterationsupport-private.h"
#include "gdx/exception.h"
#include "gdx/nodatapredicates-private.h"
#include "gdx/rasterchecks.h"
#include "gdx/rasteriterator.h"
#include "gdx/rastermetadata.h"
#include "infra/cast.h"

#include <Eigen/Core>
#include <algorithm>
#include <cassert>
#include <gsl/span>
#include <vector>

namespace gdx {

template <typename T>
class DenseRaster
{
public:
    using value_type                          = T;
    using size_type                           = int32_t;
    using data_type                           = Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
    using nodata_type                         = std::optional<value_type>;
    using pointer                             = T*;
    using const_pointer                       = const T*;
    using iterator                            = pointer;
    using const_iterator                      = const_pointer;
    static constexpr bool raster_type_has_nan = std::numeric_limits<T>::has_quiet_NaN;
    static constexpr bool with_nodata         = true;
    static constexpr T NaN                    = std::numeric_limits<T>::quiet_NaN();

    static constexpr bool typeHasNaN()
    {
        return raster_type_has_nan;
    }

    DenseRaster() = default;

    DenseRaster(int32_t rows, int32_t cols)
    : _meta(rows, cols)
    , _data(rows, cols)
    {
    }

    explicit DenseRaster(RasterMetadata meta)
    : _meta(std::move(meta))
    , _data(_meta.rows, _meta.cols)
    {
        init_nodata_values();
    }

    DenseRaster(int32_t rows, int32_t cols, T fillValue)
    : DenseRaster(RasterMetadata(rows, cols), fillValue)
    {
    }

    DenseRaster(const RasterMetadata& meta, T fillValue)
    : _meta(meta)
    , _data(meta.rows, meta.cols)
    {
        if constexpr (raster_type_has_nan) {
            // make sure we fill tha raster with NaNs if the fill value is the nodata value
            if (_meta.nodata.has_value() && fillValue == static_cast<T>(*_meta.nodata)) {
                fillValue = NaN;
            }
        }

        fill(fillValue);
    }

    DenseRaster(int32_t rows, int32_t cols, gsl::span<const T> data)
    : DenseRaster(RasterMetadata(rows, cols), data)
    {
    }

    DenseRaster(const RasterMetadata& meta, gsl::span<const T> data)
    : _meta(meta)
    , _data(meta.rows, meta.cols)
    {
        throw_on_datasize_mismatch(meta.rows, meta.cols, data.size());
        std::copy(data.begin(), data.end(), _data.data());
        init_nodata_values();
    }

    DenseRaster(const RasterMetadata& meta, data_type&& data)
    : _meta(meta)
    , _data(data)
    {
    }

    DenseRaster(const DenseRaster<T>& other)
    : _meta(other._meta)
    , _data(other._data)
    {
        fmt::print("!! Raster copy: should not happen !!");
    }

    DenseRaster(DenseRaster<T>&&) = default;

    DenseRaster& operator=(DenseRaster<T>&&) = default;

    DenseRaster& operator=(const DenseRaster<T>& other)
    {
        if (this != &other) {
            _meta = other._meta;
            _data = other._data;
        }

        return *this;
    }

    void resize_and_fill(int32_t rows, int32_t cols, value_type value)
    {
        resize(rows, cols);
        fill(value);
    }

    void resize(int32_t rows, int32_t cols)
    {
        _meta.rows = rows;
        _meta.cols = cols;
        _data.resize(rows, cols);
    }

    void resize(int32_t rows, int32_t cols, std::optional<double> nodata)
    {
        _meta.rows   = rows;
        _meta.cols   = cols;
        _meta.nodata = nodata;
        _data.resize(rows, cols);
    }

    void set_metadata(RasterMetadata meta)
    {
        if (meta.rows * meta.cols != size()) {
            throw InvalidArgument("Cannot change metadata: invalid size");
        }

        _meta = std::move(meta);
    }

    DenseRaster<T> copy() const
    {
        DenseRaster<T> dst(_meta);
        dst._data = _data;
        return dst;
    }

    auto begin()
    {
        return Eigen::begin(_data);
    }

    auto begin() const
    {
        return cbegin();
    }

    auto cbegin() const
    {
        return Eigen::cbegin(_data);
    }

    auto end()
    {
        return Eigen::end(_data);
    }

    auto end() const
    {
        return cend();
    }

    auto cend() const
    {
        return Eigen::cend(_data);
    }

    const value_type* data() const noexcept
    {
        return _data.data();
    }

    value_type* data() noexcept
    {
        return _data.data();
    }

    bool has_nodata() const noexcept
    {
        if (_meta.nodata.has_value()) {
            if constexpr (raster_type_has_nan) {
                return std::any_of(begin(), end(), [](T value) { return std::isnan(value); });
            } else {
                return std::any_of(begin(), end(), [nod = static_cast<T>(*_meta.nodata)](T value) { return value == nod; });
            }
        }

        return false;
    }

    std::optional<T> nodata() const noexcept
    {
        return inf::optional_cast<T>(_meta.nodata);
    }

    int32_t size() const noexcept
    {
        assert(_data.size() <= std::numeric_limits<int32_t>::max());
        return static_cast<int32_t>(_data.size());
    }

    void collapse_data()
    {
        // no collapse needed for non floating point types
        if constexpr (raster_type_has_nan) {
            if (_meta.nodata.has_value()) {
                std::transform(begin(), end(), begin(), [nod = inf::truncate<T>(*_meta.nodata)](T value) {
                    return std::isnan(value) ? nod : value;
                });
            }
        }
    }

    const RasterMetadata& metadata() const noexcept
    {
        return _meta;
    }

    void set_projection(int32_t epsg)
    {
        _meta.set_projection_from_epsg(epsg);
    }

    void set_nodata(double newValue)
    {
        if constexpr (!raster_type_has_nan) {
            if (std::isnan(newValue)) {
                throw InvalidArgument("Nodata value cannot be NaN for integral rasters");
            }
        }

        _meta.nodata = newValue;
    }

    void replaceNodata(T newValue)
    {
        const auto dataSize = _data.size();
        for (int i = 0; i < dataSize; ++i) {
            if (is_nodata(i)) {
                _data(i) = newValue;
            }
        }

        _meta.nodata.reset();
    }

    void turn_value_into_nodata(T value)
    {
        const auto dataSize = _data.size();
        for (int i = 0; i < dataSize; ++i) {
            if (_data(i) == value) {
                mark_as_nodata(i);
            }
        }
    }

    // assigns the value to all the elements of the raster, even nodata
    void fill(value_type value)
    {
        _data.fill(value);
    }

    // assigns the value to all the elements of the raster, leaving nodata values intact
    void fill_values(value_type value)
    {
        _data.fill(value_begin(*this), value_end(*this), value);
    }

    // Makes all elements of the raster nodata values
    void fill_with_nodata()
    {
        if (_meta.nodata.has_value()) {
            if constexpr (raster_type_has_nan) {
                fill(NaN);
            } else {
                fill(static_cast<T>(*_meta.nodata));
            }
        }
    }

    int32_t rows() const noexcept
    {
        assert(_meta.rows == _data.rows());
        return _meta.rows;
    }

    int32_t cols() const noexcept
    {
        assert(_meta.cols == _data.cols());
        return _meta.cols;
    }

    void mark_as_data(int32_t /*index*/) noexcept
    {
    }

    void mark_as_data(Cell /*cell*/) noexcept
    {
    }

    void mark_as_data(int32_t /*row*/, int32_t /*col*/) noexcept
    {
    }

    void mark_as_nodata(int32_t index)
    {
        assert(_meta.nodata.has_value());
        if (_meta.nodata.has_value()) {
            if constexpr (raster_type_has_nan) {
                _data(index) = NaN;
            } else {
                _data(index) = static_cast<T>(*_meta.nodata);
            }
        }
    }

    void mark_as_nodata(int32_t row, int32_t col)
    {
        assert(_meta.nodata.has_value());
        if (_meta.nodata.has_value()) {
            if constexpr (raster_type_has_nan) {
                _data(row, col) = NaN;
            } else {
                _data(row, col) = static_cast<T>(*_meta.nodata);
            }
        }
    }

    void mark_as_nodata(Cell cell)
    {
        mark_as_nodata(cell.r, cell.c);
    }

    std::optional<value_type> optional_value(int32_t index) const noexcept
    {
        if (is_nodata(index)) {
            return std::optional<value_type>();
        } else {
            return _data(index);
        }
    }

    template <typename VarType>
    std::optional<VarType> optional_value_as(int32_t index) const noexcept
    {
        if (is_nodata(index)) {
            return std::optional<VarType>();
        } else {
            return static_cast<VarType>(_data(index));
        }
    }

    bool is_nodataValue(T value) const noexcept
    {
        if constexpr (raster_type_has_nan) {
            return std::isnan(value);
        } else {
            if (_meta.nodata.has_value()) {
                return value == *_meta.nodata;
            } else {
                return false;
            }
        }
    }

    bool is_nodata(int32_t index) const noexcept
    {
        if (_meta.nodata.has_value()) {
            if constexpr (raster_type_has_nan) {
                return std::isnan(_data(index));
            } else {
                return _data(index) == static_cast<T>(*_meta.nodata);
            }
        }

        return false;
    }

    bool is_nodata(const Cell& cell) const noexcept
    {
        return is_nodata(cell.r, cell.c);
    }

    bool is_nodata(int32_t r, int32_t c) const noexcept
    {
        if (_meta.nodata.has_value()) {
            if constexpr (raster_type_has_nan) {
                return std::isnan(_data(r, c));
            } else {
                return _data(r, c) == static_cast<T>(*_meta.nodata);
            }
        }

        return false;
    }

    bool tolerant_equal_to(const DenseRaster<T>& other, value_type tolerance = std::numeric_limits<value_type>::epsilon()) const noexcept
    {
        if (_meta != other._meta) {
            return false;
        }

        return tolerant_data_equal_to(other, tolerance);
    }

    bool tolerant_data_equal_to(const DenseRaster<T>& other, value_type relTolerance = value_type(1e-05)) const noexcept
    {
        throw_on_size_mismatch(*this, other);

        cpu::float_equal_to<T> comp(relTolerance);

        const auto dataSize = size();
        for (int i = 0; i < dataSize; ++i) {
            if (is_nodata(i) != other.is_nodata(i)) {
                return false;
            }

            if (!is_nodata(i) && !comp(_data(i), other[i])) {
                return false;
            }
        }

        return true;
    }

    bool operator==(const DenseRaster<T>& other) const noexcept
    {
        throw_on_size_mismatch(*this, other);

        const auto dataSize = size();
        for (int i = 0; i < dataSize; ++i) {
            if (is_nodata(i) != other.is_nodata(i)) {
                return false;
            }

            if (!is_nodata(i) && (_data(i) != other[i])) {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const DenseRaster<T>& other) const noexcept
    {
        return !(*this == other);
    }

    DenseRaster<uint8_t> not_equals(const DenseRaster<T>& other) const noexcept
    {
        throw_on_size_mismatch(*this, other);
        return perform_binary_operation<nodata::not_equal_to>(other);
    }

    template <typename TValue>
    DenseRaster<uint8_t> not_equals(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return perform_unary_operation<nodata::not_equal_to>(value);
    }

    template <typename TOther>
    auto operator+(const DenseRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);
        return perform_raster_operation<std::plus>(other);
    }

    template <typename TValue>
    auto operator+(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return perform_scalar_operation<nodata::plus_scalar>(value);
    }

    DenseRaster<T>& operator+=(T value)
    {
        static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");

        cpu::plus_scalar<T> pred(value);
        for (auto& elem : _data) {
            elem = pred(elem);
        }

        return *this;
    }

    template <typename TOther>
    DenseRaster<T>& operator+=(const DenseRaster<TOther>& other)
    {
        throw_on_size_mismatch(*this, other);
        const auto dataSize = size();
        for (int32_t i = 0; i < dataSize; ++i) {
            bool leftis_nodata = is_nodata(i);

            if (leftis_nodata != other.is_nodata(i)) {
                if (leftis_nodata) {
                    mark_as_data(i);
                    _data(i) = static_cast<T>(other[i]);
                }

                continue;
            }

            _data(i) += static_cast<T>(other[i]);
        }

        return *this;
    }

    DenseRaster<T> operator-() const
    {
        if constexpr (std::is_unsigned_v<T>) {
            throw RuntimeError("Minus operator applied to unsigned value");
        } else {
            DenseRaster<T> result(_meta, DenseRaster<T>::data_type(_data));
            std::transform(result.begin(), result.end(), result.begin(), nodata::negate<T>(_meta.nodata));
            return result;
        }
    }

    template <typename TOther>
    auto operator-(const DenseRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);
        return perform_raster_operation<std::minus>(other);
    }

    template <typename TValue>
    auto operator-(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return perform_scalar_operation<nodata::minus_scalar>(value);
    }

    DenseRaster<T>& operator-=(T value)
    {
        static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");

        cpu::minus_scalar<T> pred(value);
        for (auto& elem : _data) {
            elem = pred(elem);
        }

        return *this;
    }

    template <typename TOther>
    auto operator*(const DenseRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);

        // For nan nodata, standard eigen operator can be used
        if constexpr (raster_type_has_nan && std::is_same_v<T, TOther>) {
            // all types are the same, no casts needed
            return DenseRaster<T>(_meta, _data * other._data);
        }

        return perform_raster_operation<std::multiplies>(other);
    }

    template <typename TValue>
    auto operator*(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return perform_scalar_operation<nodata::multiplies_scalar>(value);
    }

    DenseRaster<T>& operator*=(T value)
    {
        static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");

        cpu::multiplies_scalar<T> pred(value);
        for (auto& elem : _data) {
            elem = pred(elem);
        }

        return *this;
    }

    template <typename TOther>
    auto operator/(const DenseRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);

        using TResult = decltype(0.f * TOther()); // use float or double as result type
        DenseRaster<TResult> result(_meta);
        if (!_meta.nodata.has_value() && other.metadata().nodata.has_value()) {
            result.set_nodata(*other.metadata().nodata);
        }

        if (!result.nodata().has_value()) {
            result.set_nodata(std::numeric_limits<TResult>::quiet_NaN());
        }

        TResult nodata = result.nodata().value();
        if constexpr (std::numeric_limits<TResult>::has_quiet_NaN) {
            nodata = std::numeric_limits<TResult>::quiet_NaN();
        }

#pragma omp parallel for
        for (int32_t i = 0; i < size(); ++i) {
            auto v = other[i];
            if (v == 0) {
                result[i] = nodata;
            } else {
                if (is_nodata(i) || other.is_nodata(i)) {
                    result[i] = nodata;
                } else {
                    result[i] = static_cast<TResult>(_data(i)) / other[i];
                }
            }
        }

        return result;
    }

    template <typename TValue>
    auto operator/(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");

        if (value == 0) {
            throw InvalidArgument("Division by zero");
        }

        return perform_scalar_operation<nodata::divides_scalar>(value);
    }

    DenseRaster<T>& operator/=(T value)
    {
        static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");

        cpu::divides_scalar<T> pred(value);
        for (auto& elem : _data) {
            elem = pred(elem);
        }

        return *this;
    }

    value_type& operator[](int32_t index)
    {
        return _data(index);
    }

    value_type operator[](int32_t index) const
    {
        return _data(index);
    }

    value_type& operator[](const Cell& cell)
    {
        return _data(cell.r, cell.c);
    }

    const value_type& operator[](const Cell& cell) const
    {
        return _data(cell.r, cell.c);
    }

    value_type& operator()(int32_t row, int32_t col)
    {
        return _data(row, col);
    }

    const value_type& operator()(int32_t row, int32_t col) const
    {
        return _data(row, col);
    }

    DenseRaster<uint8_t> operator!() const
    {
        return perform_unary_operation<nodata::logical_not>();
    }

    template <typename TOther>
    DenseRaster<uint8_t> operator&&(const DenseRaster<TOther>& other) const
    {
        return perform_binary_operation<nodata::logical_and>(other);
    }

    template <typename TOther>
    DenseRaster<uint8_t> operator||(const DenseRaster<TOther>& other) const
    {
        return perform_binary_operation<nodata::logical_or>(other);
    }

    template <typename TOther>
    DenseRaster<uint8_t> operator>(const DenseRaster<TOther>& other) const
    {
        return perform_binary_operation<nodata::greater>(other);
    }

    DenseRaster<uint8_t> operator>(T threshold) const
    {
        return perform_unary_operation<nodata::greater>(threshold);
    }

    template <typename TOther>
    DenseRaster<uint8_t> operator>=(const DenseRaster<TOther>& other) const
    {
        return perform_binary_operation<nodata::greater_equal>(other);
    }

    DenseRaster<uint8_t> operator>=(T threshold) const
    {
        return perform_unary_operation<nodata::greater_equal>(threshold);
    }

    template <typename TOther>
    DenseRaster<uint8_t> operator<(const DenseRaster<TOther>& other) const
    {
        return perform_binary_operation<nodata::less>(other);
    }

    DenseRaster<uint8_t> operator<(T threshold) const
    {
        return perform_unary_operation<nodata::less>(threshold);
    }

    template <typename TOther>
    DenseRaster<uint8_t> operator<=(const DenseRaster<TOther>& other) const
    {
        return perform_binary_operation<nodata::less_equal>(other);
    }

    DenseRaster<uint8_t> operator<=(T threshold) const
    {
        return perform_unary_operation<nodata::less_equal>(threshold);
    }

    void replace(T oldValue, T newValue) noexcept
    {
        std::replace(begin(), end(), oldValue, newValue);
    }

    std::string to_string() const
    {
        if constexpr (std::is_same_v<uint8_t, T>) {
            DenseRaster<uint16_t> copy(_meta);
            std::copy(begin(), end(), copy.begin());
            return copy.to_string();
        } else {
            std::ostringstream ss;
            ss << _data;
            return ss.str();
        }
    }

    void init_nodata_values()
    {
        if (_meta.nodata.has_value()) {
            if constexpr (raster_type_has_nan) {
                std::replace(begin(), end(), static_cast<value_type>(*_meta.nodata), std::numeric_limits<value_type>::quiet_NaN());
            }
        }
    }

private:
    static void throw_on_datasize_mismatch(int32_t rows, int32_t cols, size_t dataSize)
    {
        if (static_cast<size_t>(rows * cols) != dataSize) {
            throw InvalidArgument("Raster data size does not match provided dimensions {} vs {}x{}", dataSize, rows, cols);
        }
    }

    // Performs a unary operation on all the elements that results in true or false
    template <template <typename> typename BinaryPredicate, typename TOther>
    DenseRaster<uint8_t> perform_unary_operation(TOther value) const
    {
        DenseRaster<uint8_t> result(_meta);
        if (_meta.nodata.has_value()) {
            result.set_nodata(static_cast<double>(std::numeric_limits<uint8_t>::max()));
        }

        auto pred       = BinaryPredicate<T>(_meta.nodata, std::optional<double>());
        const auto size = result.size();
#pragma omp parallel for
        for (int i = 0; i < size; ++i) {
            result[i] = pred(_data(i), static_cast<T>(value));
        }
        return result;
    }

    template <template <typename> typename UnaryPredicate>
    DenseRaster<uint8_t> perform_unary_operation() const
    {
        DenseRaster<uint8_t> result(_meta);
        if (_meta.nodata) {
            result.set_nodata(static_cast<double>(std::numeric_limits<uint8_t>::max()));
        }

        std::transform(cbegin(), cend(), result.begin(), UnaryPredicate<T>(_meta.nodata));
        return result;
    }

    template <template <typename> typename BinaryPredicate, typename TOther>
    DenseRaster<uint8_t> perform_binary_operation(const DenseRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);
        using WidestType = decltype(T() * TOther());

        DenseRaster<uint8_t> result(_meta);
        if (_meta.nodata.has_value() || other.metadata().nodata.has_value()) {
            result.set_nodata(std::numeric_limits<uint8_t>::max());
        }

        auto pred       = BinaryPredicate<WidestType>(_meta.nodata, other.metadata().nodata);
        const auto size = result.size();
#pragma omp parallel for
        for (int i = 0; i < size; ++i) {
            result[i] = pred(static_cast<WidestType>(_data(i)), static_cast<WidestType>(other[i]));
        }
        return result;
    }

    template <template <typename> typename UnaryPredicate, typename TScalar>
    auto perform_scalar_operation(TScalar scalar) const
    {
        using WidestType = decltype(T() * TScalar());
        auto pred        = UnaryPredicate<WidestType>(_meta.nodata, static_cast<WidestType>(scalar));
        DenseRaster<WidestType> result(_meta);
        std::transform(cbegin(), cend(), result.begin(), [this, pred](T value) {
            if (is_nodataValue(value)) {
                return value;
            }

            return pred(value);
        });
        return result;
    }

    template <template <typename> typename BinaryPredicate, typename TOther>
    auto perform_raster_operation(const DenseRaster<TOther>& other) const
    {
        using WidestType = decltype(T() * TOther());
        DenseRaster<WidestType> result(_meta);
        if (!_meta.nodata.has_value() && other.metadata().nodata.has_value()) {
            result.set_nodata(*other.metadata().nodata);
        }

        auto operation = BinaryPredicate<WidestType>();
        auto nodata    = result.nodata().value_or(0);
        if constexpr (std::numeric_limits<WidestType>::has_quiet_NaN) {
            nodata = std::numeric_limits<WidestType>::quiet_NaN();
        }

#pragma omp parallel for
        for (int32_t i = 0; i < size(); ++i) {
            if (is_nodata(i) || other.is_nodata(i)) {
                result[i] = nodata;
            } else {
                result[i] = operation(static_cast<WidestType>(_data(i)), static_cast<WidestType>(other[i]));
            }
        }

        return result;
    }

    RasterMetadata _meta;
    data_type _data;
};

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
DenseRaster<T> operator+(TScalar lhs, const DenseRaster<T>& rhs)
{
    return rhs + lhs;
}

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
auto operator-(TScalar value, const DenseRaster<T>& rhs)
{
    using ResultType = decltype(TScalar() - T());

    DenseRaster<ResultType> result(rhs.metadata());

    std::transform(begin(rhs), end(rhs), begin(result), nodata::minus_scalar_first<ResultType>(rhs.metadata().nodata, static_cast<ResultType>(value)));

    return result;
}

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
DenseRaster<T> operator*(TScalar lhs, const DenseRaster<T>& rhs)
{
    return rhs * lhs;
}

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
auto operator/(TScalar scalar, const DenseRaster<T>& rhs)
{
    //throw_on_size_mismatch(other);

    //// For nan nodata, standard eigen operator can be used
    //if constexpr (typeHasNaN() && std::is_same_v<T, TOther>) {
    //    // all types are the same, no casts needed
    //    return DenseRaster<T>(_meta, _data / other._data);
    //}

    //return performRasterOperation<nodata::divides>(other);

    using ResultType = decltype(1.0f * T());

    static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");
    DenseRaster<ResultType> result(rhs.metadata());
    for (int32_t i = 0; i < rhs.size(); ++i) {
        auto value = rhs[i];
        if (value == 0) {
            if (!result.nodata().has_value()) {
                throw InvalidArgument("Division by raster that contains 0 values");
            }

            result.mark_as_nodata(i);
        } else {
            result[i] = scalar / static_cast<ResultType>(value);
        }
    }

    return result;
}

template <typename T>
auto cbegin(const DenseRaster<T>& ras)
{
    return ras.data();
}

template <typename T>
auto cend(const DenseRaster<T>& ras)
{
    return ras.cend();
}

template <typename T>
auto begin(DenseRaster<T>& ras)
{
    return ras.begin();
}

template <typename T>
auto begin(const DenseRaster<T>& ras)
{
    return ras.begin();
}

template <typename T>
auto end(DenseRaster<T>& ras)
{
    return ras.end();
}

template <typename T>
auto end(const DenseRaster<T>& ras)
{
    return ras.cend();
}

template <typename T>
const T* data(const DenseRaster<T>& ras)
{
    return ras.data();
}

template <typename T>
T* data(DenseRaster<T>& ras)
{
    return ras.data();
}

template <typename T>
auto size(const DenseRaster<T>& ras)
{
    return ras.size();
}
}
