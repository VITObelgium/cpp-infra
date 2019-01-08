#pragma once

#include "gdx/exception.h"

#include <functional>
#include <optional>

namespace gdx::nodata {

template <class T, class U>
std::optional<T> optional_cast(std::optional<U> u)
{
    if (u.has_value()) {
        return std::make_optional<T>(static_cast<T>(*u));
    } else {
        return std::optional<T>();
    }
}

template <typename T, class Enable = void>
class float_equal_to
{
public:
    float_equal_to() = default;
    float_equal_to(std::optional<double> nodataLhs, std::optional<double> nodataRhs, T tolerance)
    : _nodataLhs(optional_cast<T>(nodataLhs))
    , _nodataRhs(optional_cast<T>(nodataRhs))
    , _tolerance(tolerance)
    {
    }

    bool operator()(T lhs, T rhs) const noexcept
    {
        if (_nodataLhs.has_value() && _nodataRhs.has_value()) {
            if (lhs == *_nodataLhs && rhs == *_nodataRhs) {
                return true;
            }
        } else if (_nodataLhs.has_value()) {
            if (lhs == *_nodataLhs) {
                // Left side is nodata, but right side does not have nodata
                return false;
            }
        } else if (_nodataRhs.has_value()) {
            if (rhs == *_nodataRhs) {
                // Right side is nodata, but left side does not have nodata
                return false;
            }
        }

        if (rhs > lhs) {
            return (rhs - lhs) <= _tolerance;
        } else {
            return (lhs - rhs) <= _tolerance;
        }
    }

private:
    std::optional<T> _nodataLhs;
    std::optional<T> _nodataRhs;
    T _tolerance = std::numeric_limits<T>::epsilon();
};

template <typename T>
class float_equal_to<T, std::enable_if_t<std::numeric_limits<T>::has_quiet_NaN>>
{
public:
    float_equal_to() = default;
    float_equal_to(std::optional<double>, std::optional<double>, T tolerance)
    : _tolerance(tolerance)
    {
    }

    bool operator()(T lhs, T rhs) const noexcept
    {
        if (std::isnan(lhs) && std::isnan(rhs)) {
            return true;
        }

        return std::fabs(rhs - lhs) <= _tolerance;
    }

private:
    T _tolerance = std::numeric_limits<T>::epsilon();
};

template <typename T, typename Operand, class Enable = void>
class raster_operation_binary_result
{
public:
    raster_operation_binary_result(const std::optional<double>& nodataLhs, const std::optional<double>& nodataRhs)
    : _nodataLhs(optional_cast<T>(nodataLhs))
    , _nodataRhs(optional_cast<T>(nodataRhs))
    {
    }

    uint8_t operator()(T lhs, T rhs) const noexcept
    {
        if (lhs == _nodataLhs) {
            return std::numeric_limits<uint8_t>::max();
        }

        if (rhs == _nodataRhs) {
            return std::numeric_limits<uint8_t>::max();
        }

        return Operand()(lhs, rhs);
    }

private:
    std::optional<T> _nodataLhs;
    std::optional<T> _nodataRhs;
};

template <typename T, typename Operand>
class raster_operation_binary_result<T, Operand, std::enable_if_t<std::numeric_limits<T>::has_quiet_NaN>>
{
public:
    raster_operation_binary_result(const std::optional<double>&, const std::optional<double>&)
    {
    }

    uint8_t operator()(T lhs, T rhs) const noexcept
    {
        if (std::isnan(lhs) || std::isnan(rhs)) {
            return std::numeric_limits<uint8_t>::max();
        }

        return Operand()(lhs, rhs);
    }
};

template <typename T, typename Operand, class Enable = void>
class raster_operation
{
public:
    raster_operation(const std::optional<double>& nodataLhs, const std::optional<double>& nodataRhs, const std::optional<double>& nodataResult = std::optional<double>())
    : _nodataLhs(optional_cast<T>(nodataLhs))
    , _nodataRhs(optional_cast<T>(nodataRhs))
    , _nodataResult(optional_cast<T>(nodataResult))
    {
        if (!_nodataResult.has_value()) {
            _nodataResult = _nodataLhs;
        }

        if (!_nodataResult.has_value()) {
            _nodataResult = _nodataRhs;
        }
    }

    T operator()(T lhs, T rhs) const noexcept
    {
        if (lhs == _nodataLhs || rhs == _nodataRhs) {
            return *_nodataResult;
        }

        return Operand()(lhs, rhs);
    }

private:
    std::optional<T> _nodataLhs;
    std::optional<T> _nodataRhs;
    std::optional<T> _nodataResult;
};

// specialisation for division which requires 0 checks
template <typename T>
class raster_operation<T, std::divides<T>, std::enable_if_t<!std::numeric_limits<T>::has_quiet_NaN>>
{
public:
    raster_operation(const std::optional<double>& nodataLhs, const std::optional<double>& nodataRhs, const std::optional<double>& nodataResult = std::optional<double>())
    : _nodataLhs(optional_cast<T>(nodataLhs))
    , _nodataRhs(optional_cast<T>(nodataRhs))
    , _nodataResult(optional_cast<T>(nodataResult))
    {
        if (!_nodataResult.has_value()) {
            _nodataResult = _nodataLhs;
        }

        if (!_nodataResult.has_value()) {
            _nodataResult = _nodataRhs;
        }
    }

    T operator()(T lhs, T rhs) const
    {
        if (lhs == _nodataLhs || rhs == _nodataRhs || rhs == 0) {
            if (!_nodataResult.has_value()) {
                throw RuntimeError("Division by zero not possible when resulting raster has no nodata");
            }

            return *_nodataResult;
        }

        return std::divides<T>()(lhs, rhs);
    }

private:
    std::optional<T> _nodataLhs;
    std::optional<T> _nodataRhs;
    std::optional<T> _nodataResult;
};

template <typename T, typename Operand, class Enable = void>
class unary_raster_operation
{
public:
    unary_raster_operation(std::optional<double> nodata)
    : _nodata(optional_cast<T>(nodata))
    {
    }

    T operator()(T value) const noexcept
    {
        if (value == _nodata) {
            return *_nodata;
        }

        return Operand()(value);
    }

private:
    std::optional<T> _nodata;
};

template <typename T, typename Operand, class Enable = void>
class unary_raster_operation_binary_result
{
public:
    unary_raster_operation_binary_result(std::optional<double> nodata)
    : _nodata(optional_cast<T>(nodata))
    {
    }

    uint8_t operator()(T value) const noexcept
    {
        if (value == _nodata) {
            return std::numeric_limits<uint8_t>::max();
        }

        return Operand()(value);
    }

private:
    std::optional<T> _nodata;
};

template <typename T, typename Operand, class Enable = void>
class unary_raster_operation_scalar
{
public:
    unary_raster_operation_scalar(std::optional<double> nodata, T scalar)
    : _scalarValue(scalar)
    , _nodata(optional_cast<T>(nodata))
    {
    }

    T operator()(T value) const noexcept
    {
        if (value == _nodata) {
            return *_nodata;
        }

        return Operand()(value, _scalarValue);
    }

private:
    T _scalarValue;
    std::optional<T> _nodata;
};

template <typename T, typename Operand, class Enable = void>
class unary_raster_operation_scalar_binary_result
{
public:
    unary_raster_operation_scalar_binary_result(std::optional<double> nodata, T scalar)
    : _scalarValue(scalar)
    , _nodata(optional_cast<T>(nodata))
    {
    }

    uint8_t operator()(T value) const noexcept
    {
        if (value == _nodata) {
            return std::numeric_limits<uint8_t>::max();
        }

        return Operand()(value, _scalarValue);
    }

private:
    T _scalarValue;
    std::optional<T> _nodata;
};

template <typename T, typename Operand>
class raster_operation<T, Operand, std::enable_if_t<std::numeric_limits<T>::has_quiet_NaN>>
{
public:
    raster_operation(const std::optional<double>&, const std::optional<double>&, const std::optional<double>& = std::optional<double>())
    {
    }

    T operator()(T lhs, T rhs) const noexcept
    {
        if (std::isnan(lhs) || std::isnan(rhs)) {
            return std::numeric_limits<T>::quiet_NaN();
        }

        return Operand()(lhs, rhs);
    }
};

template <typename T, typename Operand>
class unary_raster_operation<T, Operand, std::enable_if_t<std::numeric_limits<T>::has_quiet_NaN>>
{
public:
    unary_raster_operation(std::optional<double>)
    {
    }

    T operator()(T value) const noexcept
    {
        if (std::isnan(value)) {
            return std::numeric_limits<T>::quiet_NaN();
        }

        return Operand()(value);
    }
};

template <typename T, typename Operand>
class unary_raster_operation_binary_result<T, Operand, std::enable_if_t<std::numeric_limits<T>::has_quiet_NaN>>
{
public:
    unary_raster_operation_binary_result(std::optional<double>)
    {
    }

    uint8_t operator()(T value) const noexcept
    {
        if (std::isnan(value)) {
            return std::numeric_limits<uint8_t>::max();
        }

        return Operand()(value);
    }
};

template <typename T, typename Operand>
class unary_raster_operation_scalar_binary_result<T, Operand, std::enable_if_t<std::numeric_limits<T>::has_quiet_NaN>>
{
public:
    unary_raster_operation_scalar_binary_result(std::optional<double>, T scalar)
    : _scalarValue(scalar)
    {
    }

    uint8_t operator()(T value) const noexcept
    {
        if (std::isnan(value)) {
            return std::numeric_limits<uint8_t>::max();
        }

        return Operand()(value, _scalarValue);
    }

    T _scalarValue;
};

// specialisation for division which requires 0 checks
template <typename T>
class raster_operation<T, std::divides<T>, std::enable_if_t<std::numeric_limits<T>::has_quiet_NaN>>
{
public:
    raster_operation(const std::optional<double>&, const std::optional<double>&)
    {
    }

    T operator()(T lhs, T rhs) const noexcept
    {
        if (std::isnan(lhs) || std::isnan(rhs) || rhs == 0) {
            return std::numeric_limits<T>::quiet_NaN();
        }

        return std::divides<T>()(lhs, rhs);
    }
};

template <typename T>
using plus = raster_operation<T, std::plus<T>>;

template <typename T>
using minus = raster_operation<T, std::minus<T>>;

template <typename T>
using divides = raster_operation<T, std::divides<T>>;

template <typename T>
using multiplies = raster_operation<T, std::multiplies<T>>;

template <typename T>
using logical_and = raster_operation_binary_result<T, std::logical_and<T>>;

template <typename T>
using logical_or = raster_operation_binary_result<T, std::logical_or<T>>;

template <typename T>
using logical_not = unary_raster_operation_binary_result<T, std::logical_not<T>>;

template <typename T>
using negate = unary_raster_operation<T, std::negate<T>>;

template <typename T>
using greater = raster_operation_binary_result<T, std::greater<T>>;

template <typename T>
using less = raster_operation_binary_result<T, std::less<T>>;

template <typename T>
using greater_equal = raster_operation_binary_result<T, std::greater_equal<T>>;

template <typename T>
using less_equal = raster_operation_binary_result<T, std::less_equal<T>>;

template <typename T>
using equal_to = raster_operation_binary_result<T, std::equal_to<T>>;

template <typename T>
using not_equal_to = raster_operation_binary_result<T, std::not_equal_to<T>>;

template <typename T>
using not_equal_to_scalar = unary_raster_operation_scalar_binary_result<T, std::not_equal_to<T>>;

template <typename T>
using plus_scalar = unary_raster_operation_scalar<T, std::plus<T>>;

template <typename T>
using minus_scalar = unary_raster_operation_scalar<T, std::minus<T>>;

template <class T>
struct minus_reverse
{
    constexpr T operator()(const T& left, const T& right) const
    {
        return right - left;
    }
};

template <typename T>
using minus_scalar_first = unary_raster_operation_scalar<T, nodata::minus_reverse<T>>;

template <typename T>
using multiplies_scalar = unary_raster_operation_scalar<T, std::multiplies<T>>;

template <typename T>
using divides_scalar = unary_raster_operation_scalar<T, std::divides<T>>;
}
