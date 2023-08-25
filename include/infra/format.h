#pragma once

#include <fmt/core.h>
#include <optional>

template <typename T>
struct fmt::formatter<std::optional<T>>
{
    FMT_CONSTEXPR20 auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin();
    }

    auto format(const std::optional<T>& val, format_context& ctx) const -> format_context::iterator
    {
        if (val.has_value()) {
            return format_to(ctx.out(), "{}", *val);
        } else {
            return format_to(ctx.out(), "None");
        }
    }
};

