#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>

namespace inf {

template <typename T>
constexpr float linear_map_to_float(T value, T min, T max) noexcept
{
    if constexpr (std::is_floating_point_v<T>) {
        assert(!std::isnan(min));
        assert(!std::isnan(max));
    }
    assert(min <= max);

    if (min == max) {
        return 0.f;
    }

    if (value < min) {
        return 0.f;
    } else if (value > max) {
        return 1.f;
    }

    const auto rangeWidth = static_cast<float>(max - min);
    return static_cast<float>(value - min) / rangeWidth;
}

template <typename T>
constexpr uint8_t linear_map_to_byte(T value, T start, T end, uint8_t mapStart, uint8_t mapEnd) noexcept
{
    if (value < start || value > end) {
        return 0;
    }

    if (mapStart == mapEnd) {
        return mapStart;
    }

    const auto rangeWidth = end - start;
    const auto pos        = static_cast<float>(value - start) / static_cast<float>(rangeWidth);

    const auto mapWidth = (mapEnd - mapStart) + 1;
    const auto mapped   = std::clamp(mapStart + (mapWidth * pos), 0.f, float(std::numeric_limits<uint8_t>::max()));
    return std::clamp(static_cast<uint8_t>(mapped), mapStart, mapEnd);
}
}
