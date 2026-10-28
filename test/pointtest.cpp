#include "infra/point.h"

#include <doctest/doctest.h>
#include <limits>
#include <optional>

namespace inf::test {

TEST_CASE_TEMPLATE("Point equality", T, int, double)
{
    constexpr Point<T> first(1, 2);
    constexpr Point<T> same(1, 2);
    constexpr Point<T> differentX(2, 2);
    constexpr Point<T> differentY(1, 3);

    static_assert(first == same);
    static_assert(first != differentX);
    static_assert(first != differentY);
    CHECK(first == same);
    CHECK_FALSE(first != same);
    CHECK(first != differentX);
    CHECK(first != differentY);
}

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L && defined(__cpp_lib_three_way_comparison) && __cpp_lib_three_way_comparison >= 201907L

static_assert(std::three_way_comparable<Point<int>, std::strong_ordering>);
static_assert(std::three_way_comparable<Point<double>, std::partial_ordering>);
static_assert(std::three_way_comparable<std::optional<Point<double>>, std::partial_ordering>);
static_assert((Point<int>(1, 3) <=> Point<int>(2, 2)) == std::strong_ordering::less);
static_assert(noexcept(Point<double>(1, 2) <=> Point<double>(2, 3)));

TEST_CASE_TEMPLATE("Point comparison uses x then y", T, int, double)
{
    constexpr Point<T> first(1, 2);
    constexpr Point<T> sameX(1, 3);
    constexpr Point<T> higherX(2, 1);

    CHECK(((first <=> first) == 0));
    CHECK(((first <=> sameX) < 0));
    CHECK(((sameX <=> first) > 0));
    CHECK(((sameX <=> higherX) < 0));
    CHECK(first < sameX);
    CHECK(higherX > sameX);
    CHECK(first <= first);
    CHECK(first >= first);
}

TEST_CASE("Point comparison preserves floating point unordered results")
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const Point<double> valid(1, 2);
    for (const auto& invalid : {Point<double>(nan, 2), Point<double>(1, nan)}) {
        CHECK((invalid <=> valid) == std::partial_ordering::unordered);
        CHECK((valid <=> invalid) == std::partial_ordering::unordered);
        CHECK_FALSE(invalid < valid);
        CHECK_FALSE(invalid <= valid);
        CHECK(invalid != valid);
    }
    CHECK((Point<double>(0.0, -0.0) <=> Point<double>(-0.0, 0.0)) == std::partial_ordering::equivalent);
}

TEST_CASE("Optional Point supports defaulted containing comparisons")
{
    struct Entry
    {
        std::optional<Point<double>> coordinate;
        auto operator<=>(const Entry&) const = default;
    };

    static_assert(std::three_way_comparable<Entry, std::partial_ordering>);
    const Entry first{Point<double>(1, 2)};
    const Entry second{Point<double>(1, 3)};
    const Entry empty;

    CHECK((first <=> second) == std::partial_ordering::less);
    CHECK(first < second);
    CHECK(first == first);
    CHECK(empty < first);
    CHECK(empty == Entry{});
    CHECK((Entry{Point<double>(1, std::numeric_limits<double>::quiet_NaN())} <=> first) == std::partial_ordering::unordered);
}

#endif

}
