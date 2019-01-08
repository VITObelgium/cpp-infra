#pragma once

#include "util/exception.h"
#include "gpupolicy.h"
#include "rastermetadata.h"

#include <algorithm>
#include <vector>
#include <gsl/span>
#include <Eigen/Core>
#include <boost/compute.hpp>

namespace gdx::gpu
{

template <typename T>
class Raster
{
public:
    using value_type = T;
    using data_type = bc::vector<T>;
    using Policy = gpu::Policy<T>;
    static constexpr bool raster_type_has_nan = std::numeric_limits<T>::has_quiet_NaN;
    static constexpr T NaN = std::numeric_limits<T>::quiet_NaN();

    Raster() = default;

    Raster(int32_t rows, int32_t cols)
    : Raster(RasterMetadata(rows, cols))
    {
    }

    Raster(const RasterMetadata& meta)
    : _meta(meta)
    , _data(meta.rows * meta.cols)
    {
    }

    Raster(int32_t rows, int32_t cols, value_type fillValue)
    : Raster(RasterMetadata(rows, cols), fillValue)
    {
    }

    Raster(const RasterMetadata& meta, value_type fillValue)
    : _meta(meta)
    , _data(meta.rows * meta.cols, fillValue)
    {
    }

    Raster(int32_t rows, int32_t cols, gsl::span<const T> data)
    : Raster(RasterMetadata(rows, cols), data)
    {
    }

    Raster(const RasterMetadata& meta, gsl::span<const T> data)
    : _meta(meta)
    , _data(data.begin(), data.end())
    {
        processNodataValues();
    }

    Raster(const Raster<T>&) = delete;
    Raster(Raster<T>&&)      = default;

    Raster& operator=(Raster<T>&&) = default;
    Raster& operator=(const Raster<T>& other)
    {
        if (this != &other)
        {
            _meta = other._meta;
            resize(other.rows(), other.cols());
            copy(other.begin(), other.end(), begin());
        }

        return *this;
    }

    Raster<T> copy() const
    {
        Raster<T> dst(_meta);
        Policy::copy(begin(), end(), dst.begin());
        return dst;
    }

    auto begin() noexcept
    {
        return _data.begin();
    }

    auto end() noexcept
    {
        return _data.end();
    }

    auto begin() const noexcept
    {
        return _data.begin();
    }

    auto end() const noexcept
    {
        return _data.end();
    }

    gsl::span<value_type> span()
    {
        return _data;
    }

    gsl::span<const value_type> span() const
    {
        return _data;
    }

    std::vector<value_type> vecData() const
    {
        std::vector<value_type> hostData;
        hostData.resize(size());
        Policy::copy(_data.begin(), _data.end(), hostData.begin());
        return hostData;
    }

    auto eigen_data() const
    {
        // Copy the gpu data to an eigen array on the host
        Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> data(rows(), cols());
        gsl::span<T> dataSpan(data.data(), data.size());
        Policy::copy(_data.begin(), _data.end(), dataSpan.begin());
        return data;
    }

    int32_t size() const noexcept
    {
        assert(_data.size() <= std::numeric_limits<int32_t>::max());
        return static_cast<int32_t>(_data.size());
    }

    const RasterMetadata& metadata() const noexcept
    {
        return _meta;
    }

    void setMetaData(RasterMetadata meta)
    {
        _meta = std::move(meta);
    }

    void set_projection(int32_t epsg)
    {
        _meta.set_projectionFromEpsg(epsg);
    }

    void clear()
    {
        _data.clear();
        _meta.rows = 0;
        _meta.cols = 0;
    }

    // assigns the value to all the elements of the raster
    void fill(value_type value)
    {
        Policy::fill(_data.begin(), _data.end(), value);
    }

    void resizeAndFill(int32_t rows, int32_t cols, value_type value)
    {
        resize(rows, cols);
        fill(value);
    }

    void resize(int32_t rows, int32_t cols)
    {
        _meta.rows = rows;
        _meta.cols = cols;
        _data.resize(rows * cols);
    }

    int32_t rows() const noexcept
    {
        return _meta.rows;
    }

    int32_t cols() const noexcept
    {
        return _meta.cols;
    }

    bool is_nodata(int32_t r, int32_t c) const noexcept
    {
        if constexpr (raster_type_has_nan)
        {
            return std::isnan(*this)(r, c);
        }
        else
        {
            if (_meta.nodata.has_value())
            {
                return (*this)(r, c) == *_meta.nodata;
            }
            else
            {
                return false;
            }
        }
    }

    bool tolerantEqual(const Raster<T>& other, value_type tolerance = std::numeric_limits<value_type>::epsilon()) const noexcept
    {
        return _meta.rows == other._meta.rows &&
               _meta.cols == other._meta.cols &&
               Policy::floatEqual(_data.begin(), _data.end(), other._data.begin(), other._data.end(), tolerance);
    }

    bool operator==(const Raster<T>& other) const noexcept
    {
        return _meta.rows == other._meta.rows &&
               _meta.cols == other._meta.cols &&
               Policy::equal(_data.begin(), _data.end(), other._data.begin(), other._data.end());
    }

    Raster<T> operator==(T value) const
    {
        if constexpr (!raster_type_has_nan)
        {
            if (_meta.nodata.has_value())
            {
                auto nodata = static_cast<T>(*_meta.nodata);

                Raster<T> result(_meta);
                Policy::equalTo(_data.begin(), _data.end(), result._data.begin(), value, nodata);
                return result;
            }
        }

        Raster<T> result(_meta);
        Policy::equalTo(_data.begin(), _data.end(), result._data.begin(), value);
        return result;
    }

    bool operator!=(const Raster<T>& other) const noexcept
    {
        return !(*this == other);
    }

    operator bool() const noexcept
    {
        return size() > 0;
    }

    bool operator!() const noexcept
    {
        return !static_cast<bool>(*this);
    }

    Raster<T> operator+(const Raster<T>& other) const
    {
        return performOperator<bc::plus<T>>(other);
    }

    template <typename TValue>
    Raster<T> operator+(TValue other) const
    {
        return performOperator<bc::plus<T>>(other);
    }

    Raster<T> operator-(const Raster<T>& other) const
    {
        return performOperator<bc::minus<T>>(other);
    }

    template <typename TValue>
    Raster<T> operator-(TValue other) const
    {
        return performOperator<bc::minus<T>>(other);
    }

    Raster<T> operator*(const Raster<T>& other) const
    {
        return performOperator<bc::multiplies<T>>(other);
    }

    template <typename TValue>
    Raster<T> operator*(TValue other) const
    {
        return performOperator<bc::multiplies<T>>(other);
    }

    Raster<T> operator/(const Raster<T>& other) const
    {
        return performOperator<bc::divides<T>>(other);
    }

    template <typename TValue>
    Raster<T> operator/(TValue other) const
    {
        return performOperator<bc::divides<T>>(other);
    }

    value_type& operator[](int32_t index)
    {
        return _data[index];
    }

    value_type operator[](int32_t index) const
    {
        return _data[index];
    }

    value_type& operator()(int32_t row, int32_t col)
    {
        return _data[row * _meta.cols + col];
    }

    const value_type& operator()(int32_t row, int32_t col) const
    {
        return _data[row * _meta.cols + col];
    }

    Raster<T> operator&&(const Raster<T>& other) const
    {
        return performLogicalOperator<typename Policy::logical_and>(other);
    }

    Raster<T> operator||(const Raster<T>& other) const
    {
        return performLogicalOperator<typename Policy::logical_or>(other);
    }

    Raster<T> operator>(const Raster<T>& other) const
    {
        throw_on_size_mismatch(other);

        if constexpr (!raster_type_has_nan)
        {
            if (_meta.nodata.has_value())
            {
                auto nodata = static_cast<T>(*_meta.nodata);

                Raster<T> res(_meta);
                Policy::greaterBinary(_data.begin(), _data.end(), other._data.begin(), nodata, res._data.begin());
                return res;
            }
        }

        Raster<T> res(_meta);
        Policy::greaterBinary(_data.begin(), _data.end(), other._data.begin(), res._data.begin());
        return res;
    }

    Raster<T> operator>(value_type value) const
    {
        auto res = copy();
        Policy::greater(res.begin(), res.end(), value);
        return res;
    }

    Raster<T> operator>=(const Raster<T>& other) const
    {
        throw_on_size_mismatch(other);

        if constexpr (!raster_type_has_nan)
        {
            if (_meta.nodata.has_value())
            {
                auto nodata = static_cast<T>(*_meta.nodata);

                Raster<T> res(_meta);
                Policy::greaterEqualBinary(_data.begin(), _data.end(), other._data.begin(), nodata, res._data.begin());
                return res;
            }
        }

        Raster<T> res(_meta);
        Policy::greaterEqualBinary(_data.begin(), _data.end(), other._data.begin(), res._data.begin());
        return res;
    }

    Raster<T> operator>=(value_type value) const
    {
        auto res = copy();
        Policy::greaterEqual(res.begin(), res.end(), value);
        return res;
    }

    Raster<T> operator<(const Raster<T>& other) const
    {
        throw_on_size_mismatch(other);

        if constexpr (!raster_type_has_nan)
        {
            if (_meta.nodata.has_value())
            {
                auto nodata = static_cast<T>(*_meta.nodata);

                Raster<T> res(_meta);
                Policy::lessBinary(_data.begin(), _data.end(), other._data.begin(), nodata, res._data.begin());
                return res;
            }
        }

        Raster<T> res(_meta);
        Policy::lessBinary(_data.begin(), _data.end(), other._data.begin(), res._data.begin());
        return res;
    }

    Raster<T> operator<(value_type value) const
    {
        auto res = copy();
        Policy::less(res.begin(), res.end(), value);
        return res;
    }

    Raster<T> operator<=(const Raster<T>& other) const
    {
        throw_on_size_mismatch(other);

        if constexpr (!raster_type_has_nan)
        {
            if (_meta.nodata.has_value())
            {
                auto nodata = static_cast<T>(*_meta.nodata);

                Raster<T> res(_meta);
                Policy::lessEqualBinary(_data.begin(), _data.end(), other._data.begin(), nodata, res._data.begin());
                return res;
            }
        }

        Raster<T> res(_meta);
        Policy::lessEqualBinary(_data.begin(), _data.end(), other._data.begin(), res._data.begin());
        return res;
    }

    Raster<T> operator<=(value_type value) const
    {
        auto res = copy();
        Policy::lessEqual(res.begin(), res.end(), value);
        return res;
    }

    std::pair<int32_t, int32_t> indexToRowCol(int32_t index) const noexcept
    {
        int32_t row = index / cols();
        int32_t col = index - (row * cols());

        return { row, col };
    }

private:
    void processNodataValues()
    {
        if constexpr(raster_type_has_nan)
        {
            auto nodata = _meta.nodata;
            if (nodata.has_value())
            {
                Policy::replace(_data.begin(), _data.end(), static_cast<value_type>(*nodata), std::numeric_limits<value_type>::quiet_NaN());
            }
        }
    }

    void throwOnRasterMismatch(const Raster<T>& other) const
    {
        throw_on_size_mismatch(other);

        if constexpr (!raster_type_has_nan)
        {
            if (_meta.nodata.has_value() && other._meta.nodata.has_value())
            {
                // Both rasters have nodata, they must mattch otherwise calculations can get strange
                if (*_meta.nodata != *other._meta.nodata)
                {
                    throw InvalidArgument("Rasters have different nodata values");
                }
            }
            else if (_meta.nodata.has_value() || other._meta.nodata.has_value())
            {
                throw InvalidArgument("Rasters have different nodata values");
            }
        }
    }

    void throw_on_size_mismatch(const Raster<T>& other) const
    {
        if (other.rows() != rows() || other.cols() != cols())
        {
            throw InvalidArgument("Raster dimensions must match for operation: {}x{} vs {}x{}", rows(), cols(), other.rows(), other.cols());
        }
    }

    template<typename BinaryOperation>
    Raster<T> performOperator(const Raster<T>& other) const
    {
        throwOnRasterMismatch(other);

        Raster<T> result(_meta);
        Policy::transform(begin(), end(), other.begin(), result.begin(), BinaryOperation());
        return result;
    }

    template<typename BinaryOperation, typename TValue>
    Raster<T> performOperator(TValue value) const
    {
        Raster<T> result(_meta);

        if constexpr (raster_type_has_nan)
        {
            boost::compute::transform(begin(), end(), boost::compute::make_constant_iterator<TValue>(value), result.begin(), BinaryOperation());
        }
        else
        {
            if (_meta.nodata.has_value())
            {
                auto nodata = static_cast<T>(*_meta.nodata);
                if (std::is_same_v<boost::compute::plus<T>, BinaryOperation>)
                {
                    auto kernel = Policy::Factory::createScalarAdditionKernel(nodata);
                    boost::compute::transform(_data.begin(), _data.end(), boost::compute::make_constant_iterator(value), result.begin(), kernel);
                }
                else if (std::is_same_v<boost::compute::minus<T>, BinaryOperation>)
                {
                    auto kernel = Policy::Factory::createScalarSubtractionKernel(nodata);
                    boost::compute::transform(_data.begin(), _data.end(), boost::compute::make_constant_iterator(value), result.begin(), kernel);
                }
                else if (std::is_same_v<boost::compute::multiplies<T>, BinaryOperation>)
                {
                    auto kernel = Policy::Factory::createScalarMultipliesKernel(nodata);
                    boost::compute::transform(_data.begin(), _data.end(), boost::compute::make_constant_iterator(value), result.begin(), kernel);
                }
                else if (std::is_same_v<boost::compute::divides<T>, BinaryOperation>)
                {
                    auto kernel = Policy::Factory::createScalarDividesKernel(nodata);
                    boost::compute::transform(_data.begin(), _data.end(), boost::compute::make_constant_iterator(value), result.begin(), kernel);
                }
            }
            else
            {
                 boost::compute::transform(begin(), end(), boost::compute::make_constant_iterator<TValue>(value), result.begin(), BinaryOperation());
            }
        }

        return result;
    }

    template<typename BinaryOperation>
    Raster<T> performLogicalOperator(const Raster<T>& other) const
    {
        throw_on_size_mismatch(other);

        Raster<T> result(_meta);
        Policy::template performLogicalOperator<BinaryOperation>(begin(), end(), other.begin(), result.begin());
        return result;
    }

    RasterMetadata _meta;
    data_type _data;
};

}
