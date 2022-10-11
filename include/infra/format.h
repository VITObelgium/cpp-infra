#pragma once

#include <fmt/core.h>
#include <optional>

namespace inf {

}

namespace fmt {
template <typename T>
struct formatter<std::optional<T>>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::optional<T>& val, FormatContext& ctx) const -> decltype(ctx.out())
    {
        if (val.has_value()) {
            return format_to(ctx.out(), "{}", *val);
        } else {
            return format_to(ctx.out(), "None");
        }
    }
};
}
