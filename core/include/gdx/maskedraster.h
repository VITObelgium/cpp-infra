#pragma once

#include "cpupredicates-private.h"
#include "eigeniterationsupport-private.h"
#include "gdx/cell.h"
#include "gdx/exception.h"
#include "gdx/maskedrasteriterator.h"
#include "gdx/rasterchecks.h"
#include "gdx/rastermetadata.h"
#include "infra/cast.h"

#include <Eigen/Core>
#include <algorithm>
#include <cassert>
#include <gsl/span>
#include <vector>

namespace gdx {

using mask_type = Eigen::Array<bool, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

inline mask_type combineMask(const mask_type& lhs, const mask_type& rhs)
{
    if (rhs.size() == 0) {
        return lhs;
    }

    if (lhs.size() == 0) {
        return rhs;
    }

    assert(lhs.size() == rhs.size());
    return lhs || rhs;
}

template <typename T>
class MaskedRaster
{
public:
    using value_type                          = T;
    using mask_value_type                     = bool;
    using size_type                           = int32_t;
    using data_type                           = Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
    using mask_type                           = gdx::mask_type;
    using nodata_type                         = std::optional<value_type>;
    using pointer                             = T*;
    using const_pointer                       = const T*;
    using iterator                            = pointer;
    using const_iterator                      = const_pointer;
    static constexpr bool raster_type_has_nan = std::numeric_limits<T>::has_quiet_NaN;
    static constexpr T NaN                    = std::numeric_limits<T>::quiet_NaN();

    static constexpr bool typeHasNaN()
    {
        return raster_type_has_nan;
    }

    MaskedRaster() = default;

    MaskedRaster(int32_t rows, int32_t cols)
    : _meta(rows, cols)
    , _data(rows, cols)
    {
    }

    MaskedRaster(RasterMetadata meta)
    : _meta(std::move(meta))
    , _data(_meta.rows, _meta.cols)
    {
        if (meta.nodata.has_value()) {
            _nodataMask.resize(meta.rows, meta.cols);
            _nodataMask.fill(false);
        }
    }

    MaskedRaster(int32_t rows, int32_t cols, T fillValue)
    : MaskedRaster(RasterMetadata(rows, cols), fillValue)
    {
    }

    MaskedRaster(const RasterMetadata& meta, T fillValue)
    : _meta(meta)
    , _data(meta.rows, meta.cols)
    {
        fill(fillValue);
    }

    MaskedRaster(int32_t rows, int32_t cols, gsl::span<const T> data)
    : MaskedRaster(RasterMetadata(rows, cols), data)
    {
    }

    MaskedRaster(const RasterMetadata& meta, gsl::span<const T> data)
    : _meta(meta)
    , _data(meta.rows, meta.cols)
    {
        throw_on_datasize_mismatch(meta.rows, meta.cols, data.size());
        std::copy(data.begin(), data.end(), _data.data());
        init_nodata_values();
    }

    MaskedRaster(const RasterMetadata& meta, gsl::span<const T> data, mask_type mask)
    : _meta(meta)
    , _data(meta.rows, meta.cols)
    , _nodataMask(std::move(mask))
    {
        throw_on_datasize_mismatch(meta.rows, meta.cols, data.size());
        std::copy(data.begin(), data.end(), _data.data());
    }

    MaskedRaster(const RasterMetadata& meta, mask_type&& mask)
    : _meta(meta)
    , _data(meta.rows, meta.cols)
    , _nodataMask(mask)
    {
    }

    MaskedRaster(const RasterMetadata& meta, const mask_type& mask)
    : _meta(meta)
    , _data(meta.rows, meta.cols)
    , _nodataMask(mask)
    {
    }

    MaskedRaster(const RasterMetadata& meta, data_type&& data)
    : _meta(meta)
    , _data(data)
    {
    }

    template <typename Data, typename Mask>
    MaskedRaster(const RasterMetadata& meta, Data&& data, Mask&& mask)
    : _meta(meta)
    , _data(data)
    , _nodataMask(mask)
    {
    }

    MaskedRaster(const MaskedRaster<T>& other)
    : _meta(other._meta)
    , _data(other._data)
    , _nodataMask(other._nodataMask)
    {
        fmt::print("!! Raster copy: should not happen !!");
    }

    MaskedRaster(MaskedRaster<T>&&) = default;

    MaskedRaster& operator=(MaskedRaster<T>&&) = default;

    MaskedRaster& operator=(const MaskedRaster<T>& other)
    {
        if (this != &other) {
            _meta       = other._meta;
            _data       = other._data;
            _nodataMask = other._nodataMask;
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
        if (_nodataMask.size() != 0) {
            _nodataMask.resize(rows, cols);
        }
    }

    void resize(int32_t rows, int32_t cols, std::optional<double> nodata)
    {
        _meta.rows   = rows;
        _meta.cols   = cols;
        _meta.nodata = nodata;
        _data.resize(rows, cols);

        if (nodata.has_value()) {
            _nodataMask.resize(rows, cols);
        } else {
            _nodataMask = mask_type();
        }
    }

    void set_metadata(RasterMetadata meta)
    {
        if (meta.rows * meta.cols != size()) {
            throw InvalidArgument("Cannot change metadata: invalid size");
        }

        _meta = std::move(meta);
    }

    MaskedRaster<T> copy() const
    {
        MaskedRaster<T> dst(_meta, _nodataMask);
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

    const mask_value_type* mask() const noexcept
    {
        return _nodataMask.size() == 0 ? nullptr : _nodataMask.data();
    }

    mask_value_type* mask() noexcept
    {
        return _nodataMask.size() == 0 ? nullptr : _nodataMask.data();
    }

    bool has_nodata() const noexcept
    {
        return _nodataMask.size() != 0;
    }

    std::optional<T> nodata() const noexcept
    {
        return inf::optional_cast<T>(_meta.nodata);
    }

    const mask_type& mask_const_data() const noexcept
    {
        return _nodataMask;
    }

    mask_type& mask_data() noexcept
    {
        return _nodataMask;
    }

    const mask_type& mask_data() const noexcept
    {
        return _nodataMask;
    }

    const data_type& eigen_const_data() const noexcept
    {
        return _data;
    }

    data_type& eigen_data() noexcept
    {
        return _data;
    }

    const data_type& eigen_data() const noexcept
    {
        return _data;
    }

    int32_t size() const noexcept
    {
        assert(_data.size() <= std::numeric_limits<int32_t>::max());
        return static_cast<int32_t>(_data.size());
    }

    void collapse_data()
    {
        if (_nodataMask.size() == 0) {
            return;
        }

        assert(_meta.nodata.has_value());
        auto nodata = static_cast<value_type>(_meta.nodata.value());
        std::transform(Eigen::begin(_nodataMask), Eigen::end(_nodataMask), Eigen::begin(_data), Eigen::begin(_data), [nodata](bool mask, value_type value) {
            return mask ? nodata : value;
        });
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

        if (_meta.nodata) {
            bool nodataFill = false;
            if constexpr (raster_type_has_nan) {
                if ((std::isnan(value) && std::isnan(*_meta.nodata)) || value == _meta.nodata) {
                    _nodataMask.resize(rows(), cols());
                    nodataFill = true;
                }
            } else if (value == _meta.nodata) {
                _nodataMask.resize(rows(), cols());
                nodataFill = true;
            }

            if (!nodataFill) {
                _nodataMask = mask_type();
            } else {
                _nodataMask.fill(true);
            }
        }
    }

    // assigns the value to all the elements of the raster, leaving nodata values intact
    void fill_values(value_type value)
    {
        _data.fill(value);
    }

    // Makes all elements of the raster nodata values
    void fill_with_nodata()
    {
        _nodataMask.resize(rows(), cols());
        _nodataMask.fill(true);
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

    void mark_as_nodata(int32_t index)
    {
        if (_nodataMask.size() == 0) {
            _nodataMask.resize(rows(), cols());
            _nodataMask.fill(false);
        }
        _nodataMask(index) = true;
    }

    void mark_as_nodata(int32_t row, int32_t col)
    {
        if (_nodataMask.size() == 0) {
            _nodataMask.resize(rows(), cols());
            _nodataMask.fill(false);
        }

        _nodataMask(row, col) = true;
    }

    void mark_as_nodata(Cell cell)
    {
        mark_as_nodata(cell.r, cell.c);
    }

    void mark_as_data(int32_t index)
    {
        if (_nodataMask.size() > 0) {
            _nodataMask(index) = false;
        }
    }

    void mark_as_data(Cell cell)
    {
        if (_nodataMask.size() > 0) {
            _nodataMask(cell.r, cell.c) = false;
        }
    }

    void mark_as_data(int32_t row, int32_t col)
    {
        mark_as_data(Cell(row, col));
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

    bool is_nodata(int32_t index) const noexcept
    {
        assert(_nodataMask.size() == 0 || index < _nodataMask.size());
        return _nodataMask.size() != 0 && _nodataMask(index);
    }

    bool is_nodata(const Cell& cell) const noexcept
    {
        return is_nodata(cell.r, cell.c);
    }

    bool is_nodata(int32_t r, int32_t c) const noexcept
    {
        return _nodataMask.size() != 0 && _nodataMask(r, c);
    }

    bool tolerant_equal_to(const MaskedRaster<T>& other, value_type tolerance = std::numeric_limits<value_type>::epsilon()) const noexcept
    {
        if (_meta != other._meta) {
            return false;
        }

        return tolerant_data_equal_to(other, tolerance);
    }

    bool tolerant_data_equal_to(const MaskedRaster<T>& other, value_type relTolerance = value_type(1e-05)) const noexcept
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

    bool operator==(const MaskedRaster<T>& other) const noexcept
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

    bool operator!=(const MaskedRaster<T>& other) const noexcept
    {
        return !(*this == other);
    }

    MaskedRaster<uint8_t> not_equals(const MaskedRaster<T>& other) const noexcept
    {
        throw_on_size_mismatch(*this, other);
        return perform_binary_operation<std::not_equal_to>(other);
    }

    template <typename TValue>
    MaskedRaster<uint8_t> not_equals(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return perform_unary_operation<std::not_equal_to>(value);
    }

    template <typename TOther>
    auto operator+(const MaskedRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);
        return perform_raster_operation<std::plus>(other);
    }

    template <typename TValue>
    auto operator+(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return perform_scalar_operation<cpu::plus_scalar>(value);
    }

    MaskedRaster<T>& operator+=(T value)
    {
        static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");

        cpu::plus_scalar<T> pred(value);
        for (auto& elem : _data) {
            elem = pred(elem);
        }

        return *this;
    }

    template <typename TOther>
    MaskedRaster<T>& operator+=(const MaskedRaster<TOther>& other)
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

    MaskedRaster<T> operator-() const
    {
        if constexpr (std::is_unsigned_v<T>) {
            throw RuntimeError("Minus operator applied to unsigned value");
        } else {
            return MaskedRaster<T>(_meta, MaskedRaster<T>::data_type(-_data), _nodataMask);
        }
    }

    template <typename TOther>
    auto operator-(const MaskedRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);
        return perform_raster_operation<std::minus>(other);
    }

    template <typename TValue>
    auto operator-(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return perform_scalar_operation<cpu::minus_scalar>(value);
    }

    MaskedRaster<T>& operator-=(T value)
    {
        static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");

        cpu::minus_scalar<T> pred(value);
        for (auto& elem : _data) {
            elem = pred(elem);
        }

        return *this;
    }

    template <typename TOther>
    auto operator*(const MaskedRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);
        return perform_raster_operation<std::multiplies>(other);
    }

    template <typename TValue>
    auto operator*(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return perform_scalar_operation<cpu::multiplies_scalar>(value);
    }

    MaskedRaster<T>& operator*=(T value)
    {
        static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");

        cpu::multiplies_scalar<T> pred(value);
        for (auto& elem : _data) {
            elem = pred(elem);
        }

        return *this;
    }

    template <typename TOther>
    auto operator/(const MaskedRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);

        using TResult = decltype(0.f * TOther()); // use float or double as result type
        MaskedRaster<TResult> result(_meta, combine_mask(other));
        if (!_meta.nodata.has_value() && other.metadata().nodata.has_value()) {
            result.set_nodata(*other.metadata().nodata);
        }

        if (!result.nodata().has_value()) {
            // both rasters do not have nodata, use nan as nodata in case of division by zero
            result.set_nodata(std::numeric_limits<TResult>::quiet_NaN());
        }

        for (int32_t i = 0; i < size(); ++i) {
            auto v = other[i];
            if (v == 0 && !other.is_nodata(i)) {
                result.mark_as_nodata(i);
            } else {
                result[i] = static_cast<TResult>(_data(i)) / static_cast<TResult>(v);
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

        return perform_scalar_operation<cpu::divides_scalar>(value);
    }

    MaskedRaster<T>& operator/=(T value)
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

    MaskedRaster<uint8_t> operator!() const
    {
        return perform_unary_operation<std::logical_not>();
    }

    template <typename TOther>
    MaskedRaster<uint8_t> operator&&(const MaskedRaster<TOther>& other) const
    {
        return perform_binary_operation<std::logical_and>(other);
    }

    template <typename TOther>
    MaskedRaster<uint8_t> operator||(const MaskedRaster<TOther>& other) const
    {
        return perform_binary_operation<std::logical_or>(other);
    }

    template <typename TOther>
    MaskedRaster<uint8_t> operator>(const MaskedRaster<TOther>& other) const
    {
        return perform_binary_operation<std::greater>(other);
    }

    MaskedRaster<uint8_t> operator>(T threshold) const
    {
        return perform_unary_operation<std::greater>(threshold);
    }

    template <typename TOther>
    MaskedRaster<uint8_t> operator>=(const MaskedRaster<TOther>& other) const
    {
        return perform_binary_operation<std::greater_equal>(other);
    }

    MaskedRaster<uint8_t> operator>=(T threshold) const
    {
        return perform_unary_operation<std::greater_equal>(threshold);
    }

    template <typename TOther>
    MaskedRaster<uint8_t> operator<(const MaskedRaster<TOther>& other) const
    {
        return perform_binary_operation<std::less>(other);
    }

    MaskedRaster<uint8_t> operator<(T threshold) const
    {
        return perform_unary_operation<std::less>(threshold);
    }

    template <typename TOther>
    MaskedRaster<uint8_t> operator<=(const MaskedRaster<TOther>& other) const
    {
        return perform_binary_operation<std::less_equal>(other);
    }

    MaskedRaster<uint8_t> operator<=(T threshold) const
    {
        return perform_unary_operation<std::less_equal>(threshold);
    }

    void replace(T oldValue, T newValue) noexcept
    {
        std::replace(begin(), end(), oldValue, newValue);
    }

    void init_nodata_values()
    {
        if (_meta.nodata.has_value()) {
            _nodataMask = mask_from_data(_data, _meta.nodata);
            if constexpr (raster_type_has_nan) {
                std::replace(begin(), end(), static_cast<value_type>(*_meta.nodata), std::numeric_limits<value_type>::quiet_NaN());
            }
        }
    }

private:
    static mask_type mask_from_data(const data_type& data, std::optional<double> nodata)
    {
        mask_type mask(data.rows(), data.cols());
        if (!nodata.has_value()) {
            mask.fill(false);
            return mask;
        }

        std::transform(Eigen::begin(data), Eigen::end(data), Eigen::begin(mask), [nd = static_cast<T>(*nodata)](T value) {
            if constexpr (raster_type_has_nan) {
                return value == nd || std::isnan(value);
            } else {
                return value == nd;
            }
        });

        return mask;
    }

    template <typename TOther>
    mask_type combine_mask(const MaskedRaster<TOther>& other) const
    {
        return gdx::combineMask(mask_data(), other.mask_data());
    }

    static void throw_on_datasize_mismatch(int32_t rows, int32_t cols, size_t dataSize)
    {
        if (static_cast<size_t>(rows * cols) != dataSize) {
            throw InvalidArgument("Raster data size does not match provided dimensions {} vs {}x{}", dataSize, rows, cols);
        }
    }

    // Performs a unary operation on all the elements that results in true or false
    template <template <typename> typename BinaryPredicate, typename TOther>
    MaskedRaster<uint8_t> perform_unary_operation(TOther value) const
    {
        MaskedRaster<uint8_t> result(_meta, _nodataMask);
        if (_meta.nodata.has_value()) {
            result.set_nodata(static_cast<double>(std::numeric_limits<uint8_t>::max()));
        }

        auto pred       = BinaryPredicate<T>();
        const auto size = result.size();
#pragma omp parallel for
        for (int i = 0; i < size; ++i) {
            result[i] = pred(_data(i), static_cast<T>(value));
        }
        return result;
    }

    template <template <typename> typename UnaryPredicate>
    MaskedRaster<uint8_t> perform_unary_operation() const
    {
        MaskedRaster<uint8_t> result(_meta, _nodataMask);
        if (_meta.nodata) {
            result.set_nodata(static_cast<double>(std::numeric_limits<uint8_t>::max()));
        }

        std::transform(cbegin(), cend(), result.begin(), UnaryPredicate<T>());
        return result;
    }

    template <template <typename> typename BinaryPredicate, typename TOther>
    MaskedRaster<uint8_t> perform_binary_operation(const MaskedRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);
        using WidestType = decltype(T() * TOther());

        MaskedRaster<uint8_t> result(_meta, combine_mask(other));
        if (_meta.nodata.has_value() || other.metadata().nodata.has_value()) {
            result.set_nodata(std::numeric_limits<uint8_t>::max());
        }

        auto pred       = BinaryPredicate<WidestType>();
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
        auto pred        = UnaryPredicate<WidestType>(static_cast<WidestType>(scalar));
        MaskedRaster<WidestType> result(_meta, _nodataMask);
        std::transform(cbegin(), cend(), result.begin(), pred);
        return result;
    }

    template <template <typename> typename BinaryPredicate, typename TOther>
    auto perform_raster_operation(const MaskedRaster<TOther>& other) const
    {
        using WidestType = decltype(T() * TOther());
        MaskedRaster<WidestType> result(_meta, combine_mask(other));
        if (!_meta.nodata.has_value() && other.metadata().nodata.has_value()) {
            result.set_nodata(*other.metadata().nodata);
        }

        auto operation = BinaryPredicate<WidestType>();
#pragma omp parallel for
        for (int32_t i = 0; i < size(); ++i) {
            result[i] = operation(static_cast<WidestType>(_data(i)), static_cast<WidestType>(other[i]));
        }

        return result;
    }

    RasterMetadata _meta;
    data_type _data;
    mask_type _nodataMask;
};

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
MaskedRaster<T> operator+(TScalar lhs, const MaskedRaster<T>& rhs)
{
    return rhs + lhs;
}

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
auto operator-(TScalar value, const MaskedRaster<T>& rhs)
{
    using ResultType = decltype(TScalar() - T());

    MaskedRaster<ResultType> result(rhs.metadata(), rhs.mask_data());
    std::transform(begin(rhs), end(rhs), begin(result), [value](auto v) {
        return value - v;
    });

    return result;
}

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
MaskedRaster<T> operator*(TScalar lhs, const MaskedRaster<T>& rhs)
{
    return rhs * lhs;
}

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
auto operator/(TScalar scalar, const MaskedRaster<T>& rhs)
{
    using ResultType = decltype(1.0f * T());

    static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");
    MaskedRaster<ResultType> result(rhs.metadata(), rhs.mask_data());
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
auto cbegin(const MaskedRaster<T>& ras)
{
    return ras.data();
}

template <typename T>
auto cend(const MaskedRaster<T>& ras)
{
    return ras.cend();
}

template <typename T>
auto begin(MaskedRaster<T>& ras)
{
    return ras.begin();
}

template <typename T>
auto begin(const MaskedRaster<T>& ras)
{
    return ras.begin();
}

template <typename T>
auto end(MaskedRaster<T>& ras)
{
    return ras.end();
}

template <typename T>
auto end(const MaskedRaster<T>& ras)
{
    return ras.cend();
}

template <typename T>
const T* data(const MaskedRaster<T>& ras)
{
    return ras.data();
}

template <typename T>
T* data(MaskedRaster<T>& ras)
{
    return ras.data();
}

template <typename T>
auto size(const MaskedRaster<T>& ras)
{
    return ras.size();
}
}
