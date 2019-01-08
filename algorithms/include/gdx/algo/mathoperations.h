#pragma once

#include "gdx/algo/algorithm.h"
#include "infra/typetraits.h"

#include <cmath>

namespace gdx {

namespace internal {

template <typename ResultType, template <typename> typename RasterType, typename T, typename Op>
RasterType<ResultType> performOperation(const RasterType<T>& input, Op&& oper)
{
    RasterType<ResultType> output(input.metadata());
    gdx::transform(input, output, oper);
    return output;
}

template <typename ResultType, template <typename> typename RasterType, typename T, typename Op>
RasterType<ResultType> performOperation(const RasterType<T>& input1, const RasterType<T>& input2, Op&& oper)
{
    RasterType<ResultType> output(input1.metadata());
    gdx::transform(input1, input2, output, oper);
    return output;
}
}

template <typename RasterType>
void abs(const RasterType& input, RasterType& output)
{
    using T = typename RasterType::value_type;
    if constexpr (std::is_unsigned_v<T>) {
        (void)input;
        (void)output;
        throw RuntimeError("Absolute value on unsigned data type has no effect");
    } else {
        gdx::transform(input, output, [](auto value) {
            return std::abs(value);
        });
    }
}

template <typename RasterType>
RasterType abs(const RasterType& input)
{
    RasterType output(input.metadata());
    abs(input, output);
    return output;
}

template <typename RasterType>
void round(const RasterType& input, RasterType& output)
{
    using T = typename RasterType::value_type;
    if constexpr (!std::is_floating_point_v<T>) {
        (void)input;
        (void)output;
        throw RuntimeError("Absolute value on unsigned data type has no effect");
    } else {
        gdx::transform(input, output, [](T value) {
            return std::round(value);
        });
    }
}

template <template <typename> typename RasterType, typename T>
RasterType<T> round(const RasterType<T>& input)
{
    RasterType<T> output(input.metadata());
    round(input, output);
    return output;
}

template <typename InputRasterType, typename OutputRasterType>
void sin(const InputRasterType& input, OutputRasterType& output)
{
    using InputType  = typename InputRasterType::value_type;
    using OutputType = typename OutputRasterType::value_type;

    if constexpr (!std::is_floating_point_v<OutputType>) {
        throw RuntimeError("Sine output should be floating point");
    } else {
        gdx::transform(input, output, [](InputType value) {
            return std::sin(static_cast<OutputType>(value));
        });
    }
}

template <typename InputRasterType, typename OutputRasterType>
void cos(const InputRasterType& input, OutputRasterType& output)
{
    using InputType  = typename InputRasterType::value_type;
    using OutputType = typename OutputRasterType::value_type;

    if constexpr (!std::is_floating_point_v<OutputType>) {
        throw RuntimeError("Cosine output should be floating point");
    } else {
        gdx::transform(input, output, [](InputType value) {
            return std::cos(static_cast<OutputType>(value));
        });
    }
}

template <template <typename> typename RasterType, typename T>
auto sin(const RasterType<T>& input)
{
    if constexpr (std::is_same_v<double, T>) {
        RasterType<double> output(input.metadata());
        sin(input, output);
        return output;
    } else {
        RasterType<float> output(input.metadata());
        sin(input, output);
        return output;
    }
}

template <template <typename> typename RasterType, typename T>
auto cos(const RasterType<T>& input)
{
    if constexpr (std::is_same_v<double, T>) {
        RasterType<double> output(input.metadata());
        cos(input, output);
        return output;
    } else {
        RasterType<float> output(input.metadata());
        cos(input, output);
        return output;
    }
}

template <template <typename> typename RasterType, typename T>
RasterType<float> log(const RasterType<T>& ras)
{
    return internal::performOperation<float>(ras, [](T value) {
        return std::log(static_cast<float>(value));
    });
}

template <template <typename> typename RasterType>
RasterType<double> log(const RasterType<double>& ras)
{
    return internal::performOperation<double>(ras, [](double value) {
        return std::log(value);
    });
}

template <template <typename> typename RasterType, typename T>
RasterType<float> log10(const RasterType<T>& ras)
{
    return internal::performOperation<float>(ras, [](T value) {
        return std::log10(static_cast<float>(value));
    });
}

template <template <typename> typename RasterType>
RasterType<double> log10(const RasterType<double>& ras)
{
    return internal::performOperation<double>(ras, [](double value) {
        return std::log10(value);
    });
}

template <template <typename> typename RasterType, typename T>
RasterType<float> exp(const RasterType<T>& ras)
{
    return internal::performOperation<float>(ras, [](T value) {
        return std::exp(static_cast<float>(value));
    });
}

template <template <typename> typename RasterType>
RasterType<double> exp(const RasterType<double>& ras)
{
    return internal::performOperation<double>(ras, [](double value) {
        return std::exp(value);
    });
}

template <template <typename> typename RasterType, typename T>
auto pow(const RasterType<T>& ras1, const RasterType<T>& ras2)
{
    if constexpr (std::is_same_v<double, T>) {
        return internal::performOperation<double>(ras1, ras2, [](T value1, T value2) {
            return std::pow(value1, value2);
        });
    } else {
        return internal::performOperation<float>(ras1, ras2, [](T value1, T value2) -> float {
            return static_cast<float>(std::pow(value1, value2));
        });
    }
}

template <template <typename> typename RasterType, typename T>
void clip(const RasterType<T>& input, RasterType<T>& output, T low, T high)
{
    return gdx::transform(input, output, [low, high](auto value) {
        return std::clamp(value, low, high);
    });
}

template <template <typename> typename RasterType, typename T>
void clipLow(const RasterType<T>& input, RasterType<T>& output, T low)
{
    return gdx::transform(input, output, [low](auto value) {
        return value < low ? low : value;
    });
}

template <template <typename> typename RasterType, typename T>
void clipHigh(const RasterType<T>& input, RasterType<T>& output, T high)
{
    return gdx::transform(input, output, [high](auto value) {
        return value > high ? high : value;
    });
}

template <template <typename> typename RasterType, typename T>
RasterType<T> clip(const RasterType<T>& input, T low, T high)
{
    RasterType<T> output(input.metadata());
    clip(input, output, low, high);
    return output;
}

template <template <typename> typename RasterType, typename T>
RasterType<T> clipLow(const RasterType<T>& input, T low)
{
    RasterType<T> output(input.metadata());
    clipLow(input, output, low);
    return output;
}

template <template <typename> typename RasterType, typename T>
RasterType<T> clipHigh(const RasterType<T>& input, T high)
{
    RasterType<T> output(input.metadata());
    clipHigh(input, output, high);
    return output;
}
}
