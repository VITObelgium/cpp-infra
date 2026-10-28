#include "infra/coordinate.h"

#include <doctest/doctest.h>
#include <limits>

namespace inf::test {

TEST_CASE("Coordinate equality")
{
    constexpr Coordinate first(50.0, 4.0);
    constexpr Coordinate same(50.0, 4.0);
    constexpr Coordinate differentLatitude(51.0, 4.0);
    constexpr Coordinate differentLongitude(50.0, 5.0);

    static_assert(first == same);
    static_assert(first != differentLatitude);
    static_assert(first != differentLongitude);
    CHECK(first == same);
    CHECK_FALSE(first != same);
    CHECK(first != differentLatitude);
    CHECK(first != differentLongitude);
    CHECK(Coordinate(0.0, -0.0) == Coordinate(-0.0, 0.0));

    const Coordinate invalid;
    CHECK_FALSE(invalid == invalid);
    CHECK(invalid != invalid);
}

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L && defined(__cpp_lib_three_way_comparison) && __cpp_lib_three_way_comparison >= 201907L

static_assert(std::three_way_comparable<Coordinate, std::partial_ordering>);
static_assert((Coordinate(50.0, 5.0) <=> Coordinate(51.0, 4.0)) == std::partial_ordering::less);
static_assert(noexcept(Coordinate(50.0, 4.0) <=> Coordinate(51.0, 5.0)));

TEST_CASE("Coordinate comparison uses latitude then longitude")
{
    constexpr Coordinate first(50.0, 4.0);
    constexpr Coordinate sameLatitude(50.0, 5.0);
    constexpr Coordinate higherLatitude(51.0, 3.0);

    CHECK((first <=> first) == std::partial_ordering::equivalent);
    CHECK((first <=> sameLatitude) == std::partial_ordering::less);
    CHECK((sameLatitude <=> first) == std::partial_ordering::greater);
    CHECK((sameLatitude <=> higherLatitude) == std::partial_ordering::less);
    CHECK(first < sameLatitude);
    CHECK(higherLatitude > sameLatitude);
    CHECK(first <= first);
    CHECK(first >= first);
    CHECK((Coordinate(0.0, -0.0) <=> Coordinate(-0.0, 0.0)) == std::partial_ordering::equivalent);
}

TEST_CASE("Coordinate comparison preserves floating point unordered results")
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const Coordinate valid(50.0, 4.0);
    const Coordinate invalidLatitude(nan, 4.0);
    const Coordinate invalidLongitude(50.0, nan);

    for (const auto& invalid : {invalidLatitude, invalidLongitude, Coordinate{}}) {
        CHECK((invalid <=> valid) == std::partial_ordering::unordered);
        CHECK((valid <=> invalid) == std::partial_ordering::unordered);
        CHECK((invalid <=> invalid) == std::partial_ordering::unordered);
        CHECK_FALSE(invalid < valid);
        CHECK_FALSE(invalid > valid);
        CHECK_FALSE(invalid <= valid);
        CHECK_FALSE(invalid >= valid);
        CHECK(invalid != valid);
    }
}

TEST_CASE("Coordinate supports defaulted containing comparisons")
{
    struct Entry
    {
        Coordinate coordinate;
        auto operator<=>(const Entry&) const = default;
    };

    static_assert(std::three_way_comparable<Entry, std::partial_ordering>);
    const Entry first{Coordinate(50.0, 4.0)};
    const Entry second{Coordinate(50.0, 5.0)};

    CHECK((first <=> second) == std::partial_ordering::less);
    CHECK(first < second);
    CHECK(first == first);
    CHECK((Entry{} <=> first) == std::partial_ordering::unordered);
}

#endif

}
