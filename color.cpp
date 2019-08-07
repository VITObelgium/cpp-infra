#include "infra/color.h"

#include <fmt/format.h>

namespace inf {

std::string Color::to_hex_rgb() const noexcept
{
    return fmt::format(fmt("#{:02X}{:02X}{:02X}"), r, g, b);
}

std::string Color::to_hex_argb() const noexcept
{
    return fmt::format(fmt("#{:02X}{:02X}{:02X}{:02X}"), a, r, g, b);
}

}
