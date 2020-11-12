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

namespace fmt {
template <typename T>
struct formatter<inf::Range<T>>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const inf::Range<T>& s, FormatContext& ctx)
    {
        return format_to(ctx.out(), "[{},{}]", s.begin, s.end);
    }
};
}
