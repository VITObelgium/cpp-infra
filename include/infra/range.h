#pragma once

#include <cinttypes>
#include <fmt/core.h>

namespace inf {

template <typename T>
struct Range
{
    using range_type = T;

    constexpr Range() noexcept = default;
    constexpr Range(const T& begin_, const T& end_) noexcept
    : begin(begin_)
    , end(end_)
    {
    }

    constexpr bool operator==(const Range& other) const noexcept
    {
        return begin == other.begin && end == other.end;
    }

    constexpr bool operator!=(const Range& other) const noexcept
    {
        return !(*this == other);
    }

    constexpr bool is_valid() const noexcept
    {
        return begin <= end;
    }

    constexpr bool contains(const T& value) const noexcept
    {
        return value >= begin && value <= end;
    }

    T begin = T{};
    T end   = T{};
};

}

template <typename T>
struct fmt::formatter<inf::Range<T>>
{
    FMT_CONSTEXPR20 auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin();
    }

    auto format(const inf::Range<T>& s, format_context& ctx) const -> format_context::iterator
    {
        return fmt::format_to(ctx.out(), "[{},{}]", s.begin, s.end);
    }
};

