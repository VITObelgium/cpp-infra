#include "infra/math.h"

#include <algorithm>
#include <doctest/doctest.h>
#include <random>

namespace inf::test {

using namespace doctest;

TEST_CASE("math::Radians to degrees")
{
    CHECK(math::rad_to_deg(0.0) == 0.0);
    CHECK(math::rad_to_deg(math::pi / 6.0) == Approx(30.0));
    CHECK(math::rad_to_deg(math::pi / 4.0) == Approx(45));
    CHECK(math::rad_to_deg(math::pi / 3.0) == Approx(60));
    CHECK(math::rad_to_deg(math::pi / 2.0) == Approx(90));
    CHECK(math::rad_to_deg(2 * math::pi / 3) == Approx(120));
    CHECK(math::rad_to_deg(3 * math::pi / 4) == Approx(135));
    CHECK(math::rad_to_deg(5 * math::pi / 6) == Approx(150));
    CHECK(math::rad_to_deg(math::pi) == Approx(180));
    CHECK(math::rad_to_deg(3 * math::pi / 2) == Approx(270));
    CHECK(math::rad_to_deg(2 * math::pi) == Approx(360));
}

TEST_CASE("math::Degrees to radians")
{
    CHECK(math::deg_to_rad(0.0) == 0.0);
    CHECK(math::deg_to_rad(30.0) == Approx(math::pi / 6.0));
    CHECK(math::deg_to_rad(45.0) == Approx(math::pi / 4.0));
    CHECK(math::deg_to_rad(60.0) == Approx(math::pi / 3.0));
    CHECK(math::deg_to_rad(90.0) == Approx(math::pi / 2.0));
    CHECK(math::deg_to_rad(120.0) == Approx(2 * math::pi / 3));
    CHECK(math::deg_to_rad(135.0) == Approx(3 * math::pi / 4));
    CHECK(math::deg_to_rad(150.0) == Approx(5 * math::pi / 6));
    CHECK(math::deg_to_rad(180.0) == Approx(math::pi));
    CHECK(math::deg_to_rad(270.0) == Approx(3 * math::pi / 2));
    CHECK(math::deg_to_rad(360.0) == Approx(math::pi * 2));
    CHECK(math::deg_to_rad(-180.0) == Approx(-math::pi));
}

TEST_CASE("math::Percentiles floating point input")
{
    std::vector<double> values = {1.0, 3.0, 3.0, 4.0, 5.0, 6.0, 6.0, 7.0, 8.0, 8.0};

    SUBCASE("Sorted input")
    {
        CHECK(math::percentile_sorted_input<double>(25.0, values) == 3.0);
        CHECK(math::percentile_sorted_input<double>(50.0, values) == 6.0);
        CHECK(math::percentile_sorted_input<double>(75.0, values) == 7.0);
    }

    SUBCASE("Unsorted input")
    {
        std::random_device rd;
        std::mt19937 g(rd());

        std::shuffle(values.begin(), values.end(), g);
        CHECK(math::percentile_in_place<double>(25.0, values) == 3.0);
        std::shuffle(values.begin(), values.end(), g);
        CHECK(math::percentile_in_place<double>(50.0, values) == 6.0);
        std::shuffle(values.begin(), values.end(), g);
        CHECK(math::percentile_in_place<double>(75.0, values) == 7.0);
    }
}

TEST_CASE("math::Percentiles integral input")
{
    std::vector<int32_t> values = {1, 3, 3, 4, 5, 6, 6, 7, 8, 8};

    SUBCASE("Sorted input")
    {
        CHECK(math::percentile_sorted_input<int32_t>(25.0, values) == 3);
        CHECK(math::percentile_sorted_input<int32_t>(50.0, values) == 6);
        CHECK(math::percentile_sorted_input<int32_t>(75.0, values) == 7);
    }

    SUBCASE("Unsorted input")
    {
        std::random_device rd;
        std::mt19937 g(rd());

        std::shuffle(values.begin(), values.end(), g);
        CHECK(math::percentile_in_place<int32_t>(25.0, values) == 3);
        std::shuffle(values.begin(), values.end(), g);
        CHECK(math::percentile_in_place<int32_t>(50.0, values) == 6);
        std::shuffle(values.begin(), values.end(), g);
        CHECK(math::percentile_in_place<int32_t>(75.0, values) == 7);
    }
}

TEST_CASE("math::Mean")
{
    const std::vector<double> values = {600.0, 470.0, 170.0, 430.0, 300.0};
    CHECK(math::mean<double>(values) == 394.0);
}

TEST_CASE("math::Standard deviation")
{
    const std::vector<double> values = {600.0, 470.0, 170.0, 430.0, 300.0};

    CHECK(math::standard_deviation<double>(values) == Approx(147.3227748856));
}

}
