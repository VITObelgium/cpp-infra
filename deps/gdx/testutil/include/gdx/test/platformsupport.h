#pragma once

#include "gdxconfig.h"

#include <algorithm>
#include <vector>

namespace gdx::test {

template <typename T>
bool typeSupported()
{
    static_assert(std::is_arithmetic_v<T>, "Type should be numeric");
    return true;
}

template <typename TTo, typename TFrom>
std::vector<TTo> convertTo(const std::vector<TFrom>& from)
{
    std::vector<TTo> result(from.size());
    std::transform(from.begin(), from.end(), result.begin(), [](auto value) {
        return static_cast<TTo>(value);
    });

    return result;
}
}
