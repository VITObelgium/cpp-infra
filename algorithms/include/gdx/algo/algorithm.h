#pragma once

#include "gdx/exception.h"
#include <algorithm>
#include <optional>

#ifdef GDX_HAS_PARALLEL_ALGO
#include <execution>
#endif

namespace gdx {

/*! These transform implementations are aware of nodata values
 * If the input iterator points to nodata, the output iterator will be
 * assigned a nodata value, otherwise the provided callback is called
 * with the input value
 */

#ifdef GDX_HAS_PARALLEL_ALGO
template <class ExecutionPolicy, class InputIt, class OutputIt, class UnaryOperation, std::enable_if_t<std::is_execution_policy_v<std::decay_t<ExecutionPolicy>>, int> = 0>
OutputIt transform(ExecutionPolicy expo, InputIt first1, InputIt last1, OutputIt destFirst, UnaryOperation unaryOp)
{
    using InputType  = typename InputIt::value_type;
    using OutputType = decltype(unaryOp(std::declval<const InputType>()));

    return std::transform(expo, first1, last1, destFirst, [unaryOp](const auto& v) -> std::optional<OutputType> {
        if (v) {
            auto val = static_cast<InputType>(v);
            return unaryOp(val);
        } else {
            return std::optional<OutputType>();
        }
    });
}
#endif

template <class InputRaster, class OutputRaster, class UnaryOperation>
void transform(const InputRaster& input, OutputRaster& output, UnaryOperation unaryOp)
{
    static_assert(!std::is_const_v<OutputRaster>, "Transform output cannot be const");
    if (size(input) != size(output)) {
        throw RuntimeError("Raster sizes have to match {} vs {}", size(input), size(output));
    }

    using OutputType = typename OutputRaster::value_type;

    std::transform(optional_value_begin(input), optional_value_end(input), optional_value_begin(output), [unaryOp](const auto& v) -> std::optional<OutputType> {
        if (v) {
            return unaryOp(*v);
        }

        return std::optional<OutputType>();
    });
}

template <typename InputRaster1, typename InputRaster2, typename OutputRaster, typename BinaryOperation>
void transform(const InputRaster1& input1, const InputRaster2& input2, OutputRaster& output, BinaryOperation binaryOp)
{
    static_assert(!std::is_const_v<OutputRaster>, "Transform output cannot be const");
    if (size(input1) != size(input2)) {
        throw RuntimeError("Input raster sizes have to match {} vs {}", size(input1), size(input2));
    }

    if (size(input1) != size(output)) {
        throw RuntimeError("Input raster size has to match output size {} vs {}", size(input1), size(output));
    }

    using OutputType = typename OutputRaster::value_type;

    std::transform(optional_value_begin(input1), optional_value_end(input1), optional_value_begin(input2), optional_value_begin(output), [binaryOp](auto& v1, auto& v2) -> std::optional<OutputType> {
        if (v1 && v2) {
            return binaryOp(*v1, *v2);
        }

        return std::optional<OutputType>();
    });
}

template <typename InputRaster, typename UnaryOperation>
void for_each_data_value(const InputRaster& input, UnaryOperation unaryOp)
{
    std::for_each(value_begin(input), value_end(input), unaryOp);
}

/*! Calls the binary operation for each raster cell where both input1 and input2 have a valid value */
template <class InputRaster1, class InputRaster2, typename BinaryOperation>
void for_each_data_value(const InputRaster1& input1, const InputRaster2& input2, BinaryOperation binaryOp)
{
    using InputType1 = typename InputRaster1::value_type;
    using InputType2 = typename InputRaster2::value_type;

    if (size(input1) != size(input2)) {
        throw RuntimeError("Input raster sizes have to match {} vs {}", size(input1), size(input2));
    }

    auto iter1 = optional_value_begin(input1);
    auto iter2 = optional_value_begin(input2);
    auto last1 = optional_value_end(input1);

    for (; iter1 != last1; ++iter1, ++iter2) {
        if (!iter1->is_nodata() && !iter2->is_nodata()) {
            binaryOp(InputType1(*iter1), InputType2(*iter2));
        }
    }
}

/*! Calls the binary operation for each raster cell where both input1 and input2 have a valid value */
template <class InputRaster1, class InputRaster2, typename BinaryOperation>
void for_each_optional_value(InputRaster1& input1, InputRaster2& input2, BinaryOperation binaryOp)
{
    if (size(input1) != size(input2)) {
        throw RuntimeError("Input raster sizes have to match {} vs {}", size(input1), size(input2));
    }

    auto iter1 = optional_value_begin(input1);
    auto iter2 = optional_value_begin(input2);
    auto last1 = optional_value_end(input1);

    for (; iter1 != last1; ++iter1, ++iter2) {
        binaryOp(*iter1, *iter2);
    }
}

}
