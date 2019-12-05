#include "infra/color.h"
#include "infra/exception.h"
#include "infra/string.h"

#include <charconv>
#include <fmt/format.h>

namespace inf {

static uint8_t parse_hex(std::string_view str)
{
    uint8_t value;
    auto res = std::from_chars(str.data(), str.data() + str.size(), value, 16);
    if (res.ec == std::errc()) {
        return value;
    }

    throw InvalidArgument("Invalid hex color value: {}", str);
}

Color::Color(std::string_view hexString)
{
    if (hexString.empty()) {
        throw InvalidArgument("Empty color string");
    }

    if (hexString[0] != '#' || hexString.size() != 7 && hexString.size() != 9) {
        throw InvalidArgument("Invalid color string: {}", hexString);
    }

    int offset = 1;
    if (hexString.size() == 9) {
        a = parse_hex(hexString.substr(offset, 2));
        offset += 2;
    } else {
        a = 255;
    }

    r = parse_hex(hexString.substr(offset, 2));
    g = parse_hex(hexString.substr(offset + 2, 2));
    b = parse_hex(hexString.substr(offset + 4, 2));
}

std::string Color::to_hex_rgb() const noexcept
{
    return fmt::format(fmt("#{:02X}{:02X}{:02X}"), r, g, b);
}

std::string Color::to_hex_argb() const noexcept
{
    return fmt::format(fmt("#{:02X}{:02X}{:02X}{:02X}"), a, r, g, b);
}

}
