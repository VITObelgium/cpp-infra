#pragma once

#include "gdx/cell.h"
#include "gdx/cpupredicates-private.h"
#include "gdx/exception.h"
#include "gdx/nodatapredicates-private.h"
#include "gdx/rasterchecks.h"
#include "gdx/rastermetadata.h"
#include "gdx/sparserasteriterator.h"
#include "infra/cast.h"

#include <Eigen/SparseCore>
#include <algorithm>
#include <cassert>
#include <gsl/span>
#include <vector>

namespace gdx {

template <typename T>
class SparseRaster
{
public:
    using value_type                          = T;
    using size_type                           = int32_t;
    using data_type                           = Eigen::SparseMatrix<T, Eigen::RowMajor>;
    using nodata_type                         = std::optional<value_type>;
    using pointer                             = T*;
    using const_pointer                       = const T*;
    using iterator                            = SparseMatrixIterator<T, false>;
    using const_iterator                      = SparseMatrixIterator<T, true>;
    static constexpr bool raster_type_has_nan = std::numeric_limits<T>::has_quiet_NaN;
    static constexpr T NaN                    = std::numeric_limits<T>::quiet_NaN();

    static constexpr bool typeHasNaN()
    {
        return raster_type_has_nan;
    }

    SparseRaster() = default;

    SparseRaster(int32_t rows, int32_t cols)
    : _meta(rows, cols)
    , _data(rows, cols)
    {
    }

    SparseRaster(RasterMetadata meta)
    : _meta(std::move(meta))
    , _data(_meta.rows, _meta.cols)
    {
        throwOnInvalidMetadata();
    }

    SparseRaster(int32_t rows, int32_t cols, T fillValue)
    : SparseRaster(RasterMetadata(rows, cols), fillValue)
    {
    }

    SparseRaster(const RasterMetadata& meta, T fillValue)
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

    SparseRaster(int32_t rows, int32_t cols, gsl::span<const T> data)
    : SparseRaster(RasterMetadata(rows, cols), data)
    {
    }

    SparseRaster(const RasterMetadata& meta, gsl::span<const T> data)
    : _meta(meta)
    , _data(meta.rows, meta.cols)
    {
        throwOnInvalidMetadata();
        throwOnDataSizeMismatch(meta.rows, meta.cols, data.size());
        initMatrixValues(data);
    }

    SparseRaster(const RasterMetadata& meta, data_type&& data)
    : _meta(meta)
    , _data(data)
    {
    }

    SparseRaster(const SparseRaster<T>& other)
    : _meta(other._meta)
    , _data(other._data)
    {
        fmt::print("!! Raster copy: should not happen !!");
    }

    SparseRaster(SparseRaster<T>&&) = default;

    SparseRaster& operator=(SparseRaster<T>&&) = default;

    SparseRaster& operator=(const SparseRaster<T>& other)
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

        throwOnInvalidMetadata();
    }

    void set_metadata(RasterMetadata meta)
    {
        if (meta.rows * meta.cols != size()) {
            throw InvalidArgument("Cannot change metadata: invalid size");
        }

        _meta = std::move(meta);
    }

    SparseRaster<T> copy() const
    {
        SparseRaster<T> dst(_meta);
        dst._data = _data;
        return dst;
    }

    iterator begin()
    {
        return iterator(_data, nodata().value_or(std::numeric_limits<T>::max()));
    }

    const_iterator begin() const
    {
        return const_iterator(_data, nodata().value_or(std::numeric_limits<T>::max()));
    }

    const_iterator cbegin() const
    {
        return begin();
    }

    iterator end()
    {
        return iterator();
    }

    const_iterator end() const
    {
        return const_iterator();
    }

    const_iterator cend() const
    {
        return end();
    }

    auto value_begin()
    {
        return SparseMatrixValueIterator<value_type, false>(_data);
    }

    auto value_begin() const
    {
        return SparseMatrixValueIterator<value_type, true>(_data);
    }

    auto value_end()
    {
        return SparseMatrixValueIterator<value_type, false>();
    }

    auto value_end() const
    {
        return SparseMatrixValueIterator<value_type, true>();
    }

    auto value_cend()
    {
        return SparseMatrixValueIterator<value_type, true>();
    }

    auto value_cbegin() const
    {
        return value_begin();
    }

    auto value_cend() const
    {
        return value_end();
    }

    bool has_nodata() const noexcept
    {
        return _data.nonZeros() != 0;
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
        // no collapse needed
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

    /*void replaceNodata(double newValue)
    {
        if constexpr (!raster_type_has_nan) {
            if (std::isnan(newValue)) {
                throw InvalidArgument("Integral rasters cannot have NaN values");
            }
        }
    }*/

    /*void turn_value_into_nodata(T value)
    {
        const auto dataSize = _data.size();
        for (int i = 0; i < dataSize; ++i) {
            if (_data(i) == value) {
                mark_as_nodata(i);
            }
        }
    }*/

    // assigns the value to all the elements of the raster, even nodata
    void fill(value_type value)
    {
        // TODO
        _data.fill(value);
    }

    // assigns the value to all the elements of the raster, leaving nodata values intact
    void fill_values(value_type value)
    {
        _data.fill(value);
    }

    // Makes all elements of the raster nodata values
    void fill_with_nodata()
    {
        _data.setZero();
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
        auto [r, c] = indexToRowCol(index);
        mark_as_nodata(r, c);
    }

    void mark_as_nodata(int32_t row, int32_t col)
    {
        _data.prune([=](const int32_t& r, const int32_t& c, const T& /*value*/) {
            return row == r && col == c;
        });
    }

    std::optional<value_type> optional_value(int32_t index) const noexcept
    {
        if (is_nodata(index)) {
            return std::optional<value_type>();
        } else {
            return _data.coeff();
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
        return value == nodata();
    }

    bool is_nodata(int32_t index) const noexcept
    {
        auto [row, col] = indexToRowCol(index);
        return is_nodata(row, col);
    }

    bool is_nodata(const Cell& cell) const noexcept
    {
        return is_nodata(cell.r, cell.c);
    }

    bool is_nodata(int32_t r, int32_t c) const noexcept
    {
        auto* innerNonZeros = _data.innerNonZeroPtr();
        auto* outerIndex    = _data.outerIndexPtr();
        auto nod            = nodata().value();

        auto end = innerNonZeros ? outerIndex[r] + innerNonZeros[r] : outerIndex[r + 1];
        return _data.data().atInRange(outerIndex[r], end, typename data_type::StorageIndex(c), nod) == nod;
    }

    // bool tolerant_equal_to(const SparseRaster<T>& other, value_type tolerance = std::numeric_limits<value_type>::epsilon()) const noexcept
    // {
    //     if (_meta != other._meta) {
    //         return false;
    //     }

    //     return tolerant_data_equal_to(other, tolerance);
    // }

    // bool tolerant_data_equal_to(const SparseRaster<T>& other, value_type relTolerance = value_type(1e-05)) const noexcept
    // {
    //     throw_on_size_mismatch(*this, other);

    //     return _data == other._data;
    // }

    bool operator==(const SparseRaster<T>& other) const noexcept
    {
        throw_on_size_mismatch(*this, other);
        return (_data - other._data).norm() == 0;
    }

    bool operator!=(const SparseRaster<T>& other) const noexcept
    {
        return !(*this == other);
    }

    /*SparseRaster<uint8_t> not_equals(const SparseRaster<T>& other) const noexcept
    {
        throw_on_size_mismatch(*this, other);
        return performBinaryOperation<nodata::not_equal_to>(other);
    }

    template <typename TValue>
    SparseRaster<uint8_t> not_equals(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return performUnaryOperation<nodata::not_equal_to>(value);
    }*/

    template <typename TOther>
    auto operator+(const SparseRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);
        return SparseRaster(_meta, _data + other._data);
    }

    template <typename TValue>
    auto operator+(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        SparseRaster rasterCopy = copy();
        rasterCopy += value;
        return rasterCopy;
    }

    SparseRaster<T>& operator+=(T value)
    {
        static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");
        std::for_each(value_begin(), value_end(), [=](T& cellValue) {
            cellValue += static_cast<T>(value);
        });

        return *this;
    }

    template <typename TOther>
    SparseRaster<T>& operator+=(const SparseRaster<TOther>& other)
    {
        _data += other._data;
        return *this;
    }

    SparseRaster<T> operator-() const
    {
        if constexpr (std::is_unsigned_v<T>) {
            throw RuntimeError("Minus operator applied to unsigned value");
        } else {
            return SparseRaster<T>(_meta, -_data);
        }
    }

    template <typename TOther>
    auto operator-(const SparseRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);
        return SparseRaster<T>(_meta, _data - other._data);
    }

    template <typename TValue>
    auto operator-(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        SparseRaster rasterCopy = copy();
        rasterCopy -= value;
        return rasterCopy;
    }

    SparseRaster<T>& operator-=(T value)
    {
        static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");
        std::for_each(value_begin(), value_end(), [=](T& cellValue) {
            cellValue -= static_cast<T>(value);
        });
        return *this;
    }

    template <typename TOther>
    auto operator*(const SparseRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);
        return SparseRaster<T>(_meta, _data * other._data);
    }

    template <typename TValue>
    auto operator*(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return SparseRaster(_meta, _data * static_cast<T>(value));
    }

    SparseRaster<T>& operator*=(T value)
    {
        static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");
        _data *= value;
        return *this;
    }

    //template <typename TOther>
    //auto operator/(const SparseRaster<TOther>& other) const
    //{
    //    throw_on_size_mismatch(*this, other);

    //    using TResult = decltype(0.f * TOther()); // use float or double as result type
    //    SparseRaster<TResult> result(_meta);
    //    if (!_meta.nodata.has_value() && other.metadata().nodata.has_value()) {
    //        result.set_nodata(*other.metadata().nodata);
    //    }

    //    if (!result.nodata().has_value()) {
    //        result.set_nodata(std::numeric_limits<TResult>::quiet_NaN());
    //    }

    //    TResult nodata = result.nodata().value();
    //    if constexpr (std::numeric_limits<TResult>::has_quiet_NaN) {
    //        nodata = std::numeric_limits<TResult>::quiet_NaN();
    //    }

    //    auto operation = nodata::divides<TResult>(_meta.nodata, other.metadata().nodata);

    //    for (int32_t i = 0; i < size(); ++i) {
    //        auto v = other[i];
    //        if (v == 0) {
    //            result[i] = nodata;
    //        } else {
    //            if (is_nodata(i) || other.is_nodata(i)) {
    //                result[i] = nodata;
    //            } else {
    //                result[i] = static_cast<TResult>(_data(i)) / other[i];
    //            }
    //        }
    //    }

    //    return result;
    //}

    template <typename TValue>
    auto operator/(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");

        if (value == 0) {
            throw InvalidArgument("Division by zero");
        }

        return SparseRaster(_meta, _data / static_cast<T>(value));
    }

    SparseRaster<T>& operator/=(T value)
    {
        static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");
        _data /= value;
        return *this;
    }

    value_type& operator[](int32_t index)
    {
        auto [row, col] = indexToRowCol(index);
        return (*this)(row, col);
    }

    value_type operator[](int32_t index) const
    {
        auto [row, col] = indexToRowCol(index);
        return (*this)(row, col);
    }

    value_type& operator[](const Cell& cell)
    {
        return (*this)(cell.r, cell.c);
    }

    value_type operator[](const Cell& cell) const
    {
        return (*this)(cell.r, cell.c);
    }

    value_type& operator()(int32_t row, int32_t col)
    {
        return _data.coeffRef(row, col);
    }

    value_type operator()(int32_t row, int32_t col) const
    {
        return _data.coeff(row, col);
    }

    /*SparseRaster<uint8_t> operator!() const
    {
        return performUnaryOperation<nodata::logical_not>();
    }

    template <typename TOther>
    SparseRaster<uint8_t> operator&&(const SparseRaster<TOther>& other) const
    {
        return performBinaryOperation<nodata::logical_and>(other);
    }

    template <typename TOther>
    SparseRaster<uint8_t> operator||(const SparseRaster<TOther>& other) const
    {
        return performBinaryOperation<nodata::logical_or>(other);
    }

    template <typename TOther>
    SparseRaster<uint8_t> operator>(const SparseRaster<TOther>& other) const
    {
        return performBinaryOperation<nodata::greater>(other);
    }

    SparseRaster<uint8_t> operator>(T threshold) const
    {
        return performUnaryOperation<nodata::greater>(threshold);
    }

    template <typename TOther>
    SparseRaster<uint8_t> operator>=(const SparseRaster<TOther>& other) const
    {
        return performBinaryOperation<nodata::greater_equal>(other);
    }

    SparseRaster<uint8_t> operator>=(T threshold) const
    {
        return performUnaryOperation<nodata::greater_equal>(threshold);
    }

    template <typename TOther>
    SparseRaster<uint8_t> operator<(const SparseRaster<TOther>& other) const
    {
        return performBinaryOperation<nodata::less>(other);
    }

    SparseRaster<uint8_t> operator<(T threshold) const
    {
        return performUnaryOperation<nodata::less>(threshold);
    }

    template <typename TOther>
    SparseRaster<uint8_t> operator<=(const SparseRaster<TOther>& other) const
    {
        return performBinaryOperation<nodata::less_equal>(other);
    }

    SparseRaster<uint8_t> operator<=(T threshold) const
    {
        return performUnaryOperation<nodata::less_equal>(threshold);
    }*/

    void replace(T oldValue, T newValue) noexcept
    {
        std::replace(begin(), end(), oldValue, newValue);
    }

    std::string to_string() const
    {
        std::ostringstream ss;
        ss << _data;
        return ss.str();
    }

private:
    std::tuple<int32_t, int32_t> indexToRowCol(int32_t index) const
    {
        int row = index / inf::truncate<int>(_data.cols());
        int col = index - (row * inf::truncate<int>(_data.cols()));

        return {row, col};
    }

    void initMatrixValues(gsl::span<const T> data)
    {
        assert(nodata().has_value());

        const T nod = nodata().value();

        std::vector<Eigen::Triplet<T>> tripletList;
        for (int r = 0; r < _meta.rows; ++r) {
            const int rowStart = r * _meta.cols;
            for (int c = 0; c < _meta.cols; ++c) {
                if (data[rowStart + c] != nod) {
                    tripletList.push_back(Eigen::Triplet<T>(r, c, data[rowStart + c]));
                }
            }
        }

        _data.setFromTriplets(tripletList.begin(), tripletList.end());
    }

    void throwOnInvalidMetadata()
    {
        if (!_meta.nodata.has_value()) {
            throw RuntimeError("Sparse rasters must have a nodata value");
        }
    }

    static void throwOnDataSizeMismatch(int32_t rows, int32_t cols, size_t dataSize)
    {
        if (static_cast<size_t>(rows * cols) != dataSize) {
            throw InvalidArgument("Raster data size does not match provided dimensions {} vs {}x{}", dataSize, rows, cols);
        }
    }

    // Performs a unary operation on all the elements that results in true or false
    template <template <typename> typename BinaryPredicate, typename TOther>
    SparseRaster<uint8_t> perform_unary_operation(TOther value) const
    {
        SparseRaster<uint8_t> result(_meta);
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
    SparseRaster<uint8_t> performUnaryOperation() const
    {
        SparseRaster<uint8_t> result(_meta);
        if (_meta.nodata) {
            result.set_nodata(static_cast<double>(std::numeric_limits<uint8_t>::max()));
        }

        std::transform(cbegin(), cend(), result.begin(), UnaryPredicate<T>(_meta.nodata));
        return result;
    }

    template <template <typename> typename BinaryPredicate, typename TOther>
    SparseRaster<uint8_t> perform_binary_operation(const SparseRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);
        using WidestType = decltype(T() * TOther());

        SparseRaster<uint8_t> result(_meta);
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
        SparseRaster<WidestType> result(_meta);
        std::transform(cbegin(), cend(), result.begin(), [this, pred](T value) {
            if (is_nodataValue(value)) {
                return value;
            }

            return pred(value);
        });
        return result;
    }

    template <template <typename> typename BinaryPredicate, typename TOther>
    auto perform_raster_operation(const SparseRaster<TOther>& other) const
    {
        using WidestType = decltype(T() * TOther());
        SparseRaster<WidestType> result(_meta);
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
SparseRaster<T> operator+(TScalar lhs, const SparseRaster<T>& rhs)
{
    return rhs + lhs;
}

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
auto operator-(TScalar value, const SparseRaster<T>& rhs)
{
    using ResultType = decltype(TScalar() - T());

    SparseRaster<ResultType> result(rhs.metadata());

    std::transform(begin(rhs), end(rhs), begin(result), nodata::minus_scalar_first<ResultType>(rhs.metadata().nodata, static_cast<ResultType>(value)));

    return result;
}

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
SparseRaster<T> operator*(TScalar lhs, const SparseRaster<T>& rhs)
{
    return rhs * lhs;
}

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
auto operator/(TScalar scalar, const SparseRaster<T>& rhs)
{
    //throw_on_size_mismatch(other);

    //// For nan nodata, standard eigen operator can be used
    //if constexpr (typeHasNaN() && std::is_same_v<T, TOther>) {
    //    // all types are the same, no casts needed
    //    return SparseRaster<T>(_meta, _data / other._data);
    //}

    //return performRasterOperation<nodata::divides>(other);

    using ResultType = decltype(1.0f * T());

    static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");
    SparseRaster<ResultType> result(rhs.metadata());
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
auto cbegin(const SparseRaster<T>& ras)
{
    return ras.data();
}

template <typename T>
auto cend(const SparseRaster<T>& ras)
{
    return ras.cend();
}

template <typename T>
auto begin(SparseRaster<T>& ras)
{
    return ras.begin();
}

template <typename T>
auto begin(const SparseRaster<T>& ras)
{
    return ras.begin();
}

template <typename T>
auto end(SparseRaster<T>& ras)
{
    return ras.end();
}

template <typename T>
auto end(const SparseRaster<T>& ras)
{
    return ras.cend();
}

template <typename T>
auto size(const SparseRaster<T>& ras)
{
    return ras.size();
}

template <typename T>
auto value_cbegin(const SparseRaster<T>& ras)
{
    return ras.value_data();
}

template <typename T>
auto value_cend(const SparseRaster<T>& ras)
{
    return ras.value_cend();
}

template <typename T>
auto value_begin(SparseRaster<T>& ras)
{
    return ras.value_begin();
}

template <typename T>
auto value_begin(const SparseRaster<T>& ras)
{
    return ras.value_begin();
}

template <typename T>
auto value_end(SparseRaster<T>& ras)
{
    return ras.value_end();
}

template <typename T>
auto value_end(const SparseRaster<T>& ras)
{
    return ras.value_cend();
}
}
