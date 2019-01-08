#pragma once

#include "gdx/exception.h"

#include <functional>
#include <optional>

namespace gdx::cpu {

template <typename T, class Enable = void>
class float_equal_to
{
public:
    float_equal_to(T = 0, T = 0)
    {
    }

    bool operator()(T lhs, T rhs) const noexcept
    {
        return lhs == rhs;
    }
};

template <typename T>
class float_equal_to<T, std::enable_if_t<std::is_floating_point_v<T>>>
{
public:
    float_equal_to(T relTolerance = T(1e-05), T absTolerance = T(1e-08))
    : _relTolerance(relTolerance)
    , _absTolerance(absTolerance)
    {
    }

    bool operator()(T lhs, T rhs) const noexcept
    {
        return std::abs(lhs - rhs) <= (_absTolerance + _relTolerance * std::abs(rhs));
    }

private:
    T _relTolerance;
    T _absTolerance;
};

template <typename T, typename Operand>
class unary_raster_operation_scalar
{
public:
    unary_raster_operation_scalar(T scalar)
    : _scalarValue(scalar)
    {
    }

    T operator()(T value) const noexcept
    {
        return Operand()(value, _scalarValue);
    }

private:
    T _scalarValue;
};

template <typename T, typename Operand>
class unary_raster_operation_scalar_binary_result
{
public:
    unary_raster_operation_scalar_binary_result(T scalar)
    : _scalarValue(scalar)
    {
    }

    uint8_t operator()(T value) const noexcept
    {
        return Operand()(value, _scalarValue);
    }

private:
    T _scalarValue;
};

template <typename T>
using equal_to_scalar = unary_raster_operation_scalar_binary_result<T, std::equal_to<T>>;

template <typename T>
using not_equal_to_scalar = unary_raster_operation_scalar_binary_result<T, std::not_equal_to<T>>;

template <typename T>
using plus_scalar = unary_raster_operation_scalar<T, std::plus<T>>;

template <typename T>
using minus_scalar = unary_raster_operation_scalar<T, std::minus<T>>;

template <typename T>
using multiplies_scalar = unary_raster_operation_scalar<T, std::multiplies<T>>;

template <typename T>
using divides_scalar = unary_raster_operation_scalar<T, std::divides<T>>;
}
