#include "infra/color.h"

namespace infra {

Color::Color(uint8_t r_, uint8_t g_, uint8_t b_) noexcept
: Color(r_, g_, b_, 255)
{
}

Color::Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) noexcept
: r(r_)
, g(g_)
, b(b_)
, a(a_)
{
}

bool Color::operator==(const Color& other) const noexcept
{
    return r == other.r &&
           g == other.g &&
           b == other.b &&
           a == other.a;
}
}
