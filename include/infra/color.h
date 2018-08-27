#pragma once

#include <cinttypes>

namespace inf {

struct Color
{
    Color() = default;
    Color(uint8_t r_, uint8_t g_, uint8_t b_) noexcept;
    Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) noexcept;

    bool operator==(const Color& other) const noexcept;

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;
};
}
