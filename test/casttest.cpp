#include "infra/cast.h"

#include <cmath>

#include <gtest/gtest.h>

namespace inf::test {

using namespace testing;

TEST(CastTest, fits_in_type)
{
    constexpr double minFloat  = std::numeric_limits<float>::lowest();
    constexpr double maxFloat  = std::numeric_limits<float>::max();
    constexpr double minDouble = std::numeric_limits<double>::lowest();
    constexpr double maxDouble = std::numeric_limits<double>::max();

    static_assert(fits_in_type<uint8_t>(255));
    static_assert(!fits_in_type<uint8_t>(-1));
    static_assert(!fits_in_type<uint8_t>(256));

    static_assert(fits_in_type<uint8_t>(255.0));

    static_assert(fits_in_type<float>(0.f));
    static_assert(fits_in_type<float>(minFloat));
    static_assert(fits_in_type<float>(maxFloat));

    static_assert(fits_in_type<int32_t>(uint16_t(0)));
    static_assert(fits_in_type<int32_t>(std::numeric_limits<uint16_t>::max()));

    // maximum values
    static_assert(fits_in_type<uint8_t>(uint64_t(std::numeric_limits<uint8_t>::max())));
    static_assert(fits_in_type<uint16_t>(uint64_t(std::numeric_limits<uint16_t>::max())));
    static_assert(fits_in_type<uint32_t>(uint64_t(std::numeric_limits<uint32_t>::max())));
    static_assert(fits_in_type<uint64_t>(std::numeric_limits<uint64_t>::max()));

    static_assert(fits_in_type<int8_t>(int64_t(std::numeric_limits<int8_t>::max())));
    static_assert(fits_in_type<int16_t>(int64_t(std::numeric_limits<int16_t>::max())));
    static_assert(fits_in_type<int32_t>(int64_t(std::numeric_limits<int32_t>::max())));
    static_assert(fits_in_type<int64_t>(std::numeric_limits<int64_t>::max()));

    EXPECT_FALSE(fits_in_type<float>(std::nextafter(maxFloat, maxDouble)));

    if constexpr (sizeof(long double) != sizeof(double)) {
        EXPECT_FALSE(fits_in_type<double>(std::nextafter(maxDouble, std::numeric_limits<long double>::max())));
    }

    // maximum values + 1
    static_assert(!fits_in_type<uint8_t>(std::numeric_limits<uint8_t>::max() + 1));
    static_assert(!fits_in_type<uint16_t>(std::numeric_limits<uint16_t>::max() + 1));
    static_assert(!fits_in_type<uint32_t>(std::numeric_limits<uint32_t>::max() + 1ll));

    static_assert(!fits_in_type<int8_t>(std::numeric_limits<int8_t>::max() + 1));
    static_assert(!fits_in_type<int16_t>(std::numeric_limits<int16_t>::max() + 1));
    static_assert(!fits_in_type<int32_t>(std::numeric_limits<int32_t>::max() + 1ll));

    EXPECT_FALSE(fits_in_type<float>(std::nextafter(maxFloat, maxDouble)));
    if constexpr (sizeof(long double) != sizeof(double)) {
        EXPECT_FALSE(fits_in_type<double>(std::nextafter(maxDouble, std::numeric_limits<long double>::max())));
    }

    // minimum values
    static_assert(fits_in_type<int8_t>(int64_t(std::numeric_limits<int8_t>::lowest())));
    static_assert(fits_in_type<int16_t>(int64_t(std::numeric_limits<int16_t>::lowest())));
    static_assert(fits_in_type<int32_t>(int64_t(std::numeric_limits<int32_t>::lowest())));
    static_assert(fits_in_type<int64_t>(std::numeric_limits<int64_t>::lowest()));

    static_assert(fits_in_type<float>(std::numeric_limits<float>::lowest()));
    static_assert(fits_in_type<double>(std::numeric_limits<double>::lowest()));

    // minimum values - 1
    static_assert(!fits_in_type<int8_t>(int64_t(std::numeric_limits<int8_t>::lowest() - 1ll)));
    static_assert(!fits_in_type<int16_t>(int64_t(std::numeric_limits<int16_t>::lowest() - 1ll)));
    static_assert(!fits_in_type<int32_t>(int64_t(std::numeric_limits<int32_t>::lowest() - 1ll)));

    EXPECT_FALSE(fits_in_type<float>(std::nextafter(minFloat, minDouble)));
    if constexpr (sizeof(long double) != sizeof(double)) {
        EXPECT_FALSE(fits_in_type<double>(std::nextafter(minDouble, std::numeric_limits<long double>::lowest())));
    }

    // negative values in unsigned types
    static_assert(!fits_in_type<uint8_t>(-1));
    static_assert(!fits_in_type<uint16_t>(-1));
    static_assert(!fits_in_type<uint32_t>(-1));
    static_assert(!fits_in_type<uint64_t>(-1));

    static_assert(!fits_in_type<uint8_t>(-1.0));
    static_assert(!fits_in_type<uint16_t>(-1.0));
    static_assert(!fits_in_type<uint32_t>(-1.0));
    static_assert(!fits_in_type<uint64_t>(-1.0));

    static_assert(!fits_in_type<uint32_t>(int16_t(-1)));
    static_assert(!fits_in_type<uint32_t>(int32_t(-1)));
    static_assert(!fits_in_type<uint32_t>(int64_t(-1)));

    // signed unsigned same size
    static_assert(!fits_in_type<int8_t>(std::numeric_limits<uint8_t>::max()));
    static_assert(!fits_in_type<int16_t>(std::numeric_limits<uint16_t>::max()));
    static_assert(!fits_in_type<int32_t>(std::numeric_limits<uint32_t>::max()));
    static_assert(!fits_in_type<int64_t>(std::numeric_limits<uint64_t>::max()));

    // signed unsigned different size
    static_assert(!fits_in_type<int8_t>(std::numeric_limits<uint16_t>::max()));
    static_assert(!fits_in_type<int16_t>(std::numeric_limits<uint32_t>::max()));
    static_assert(!fits_in_type<int32_t>(std::numeric_limits<uint64_t>::max()));

    // floating point checks
    static_assert(fits_in_type<double>(std::numeric_limits<uint64_t>::max()));
    static_assert(!fits_in_type<uint64_t>(std::numeric_limits<double>::max()));

    static_assert(fits_in_type<uint8_t>(255.0));
    static_assert(!fits_in_type<uint8_t>(255.1));

    EXPECT_FALSE(fits_in_type<uint64_t>(std::nextafter(std::numeric_limits<uint64_t>::max(), maxDouble)));
}
}
