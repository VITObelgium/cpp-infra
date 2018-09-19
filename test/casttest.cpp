#include "infra/cast.h"

#include <cmath>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace inf::test {

using namespace testing;

TEST(CastTest, fitsInType)
{
    constexpr double minFloat  = std::numeric_limits<float>::lowest();
    constexpr double maxFloat  = std::numeric_limits<float>::max();
    constexpr double minDouble = std::numeric_limits<double>::lowest();
    constexpr double maxDouble = std::numeric_limits<double>::max();

    static_assert(fitsInType<uint8_t>(255));
    static_assert(!fitsInType<uint8_t>(-1));
    static_assert(!fitsInType<uint8_t>(256));

    static_assert(fitsInType<uint8_t>(255.0));

    static_assert(fitsInType<float>(0.f));
    static_assert(fitsInType<float>(minFloat));
    static_assert(fitsInType<float>(maxFloat));

    static_assert(fitsInType<int32_t>(uint16_t(0)));
    static_assert(fitsInType<int32_t>(std::numeric_limits<uint16_t>::max()));

    // maximum values
    static_assert(fitsInType<uint8_t>(uint64_t(std::numeric_limits<uint8_t>::max())));
    static_assert(fitsInType<uint16_t>(uint64_t(std::numeric_limits<uint16_t>::max())));
    static_assert(fitsInType<uint32_t>(uint64_t(std::numeric_limits<uint32_t>::max())));
    static_assert(fitsInType<uint64_t>(std::numeric_limits<uint64_t>::max()));

    static_assert(fitsInType<int8_t>(int64_t(std::numeric_limits<int8_t>::max())));
    static_assert(fitsInType<int16_t>(int64_t(std::numeric_limits<int16_t>::max())));
    static_assert(fitsInType<int32_t>(int64_t(std::numeric_limits<int32_t>::max())));
    static_assert(fitsInType<int64_t>(std::numeric_limits<int64_t>::max()));

    EXPECT_FALSE(fitsInType<float>(std::nextafter(maxFloat, maxDouble)));
    EXPECT_FALSE(fitsInType<double>(std::nextafter(maxDouble, std::numeric_limits<long double>::max())));

    // maximum values + 1
    static_assert(!fitsInType<uint8_t>(std::numeric_limits<uint8_t>::max() + 1));
    static_assert(!fitsInType<uint16_t>(std::numeric_limits<uint16_t>::max() + 1));
    static_assert(!fitsInType<uint32_t>(std::numeric_limits<uint32_t>::max() + 1ll));

    static_assert(!fitsInType<int8_t>(std::numeric_limits<int8_t>::max() + 1));
    static_assert(!fitsInType<int16_t>(std::numeric_limits<int16_t>::max() + 1));
    static_assert(!fitsInType<int32_t>(std::numeric_limits<int32_t>::max() + 1ll));

    EXPECT_FALSE(fitsInType<float>(std::nextafter(maxFloat, maxDouble)));
    EXPECT_FALSE(fitsInType<double>(std::nextafter(maxDouble, std::numeric_limits<long double>::max())));

    // minimum values
    static_assert(fitsInType<int8_t>(int64_t(std::numeric_limits<int8_t>::lowest())));
    static_assert(fitsInType<int16_t>(int64_t(std::numeric_limits<int16_t>::lowest())));
    static_assert(fitsInType<int32_t>(int64_t(std::numeric_limits<int32_t>::lowest())));
    static_assert(fitsInType<int64_t>(std::numeric_limits<int64_t>::lowest()));

    static_assert(fitsInType<float>(std::numeric_limits<float>::lowest()));
    static_assert(fitsInType<double>(std::numeric_limits<double>::lowest()));

    // minimum values - 1
    static_assert(!fitsInType<int8_t>(int64_t(std::numeric_limits<int8_t>::lowest() - 1ll)));
    static_assert(!fitsInType<int16_t>(int64_t(std::numeric_limits<int16_t>::lowest() - 1ll)));
    static_assert(!fitsInType<int32_t>(int64_t(std::numeric_limits<int32_t>::lowest() - 1ll)));

    EXPECT_FALSE(fitsInType<float>(std::nextafter(minFloat, minDouble)));
    EXPECT_FALSE(fitsInType<double>(std::nextafter(minDouble, std::numeric_limits<long double>::lowest())));

    // negative values in unsigned types
    static_assert(!fitsInType<uint8_t>(-1));
    static_assert(!fitsInType<uint16_t>(-1));
    static_assert(!fitsInType<uint32_t>(-1));
    static_assert(!fitsInType<uint64_t>(-1));

    static_assert(!fitsInType<uint8_t>(-1.0));
    static_assert(!fitsInType<uint16_t>(-1.0));
    static_assert(!fitsInType<uint32_t>(-1.0));
    static_assert(!fitsInType<uint64_t>(-1.0));

    static_assert(!fitsInType<uint32_t>(int16_t(-1)));
    static_assert(!fitsInType<uint32_t>(int32_t(-1)));
    static_assert(!fitsInType<uint32_t>(int64_t(-1)));

    // signed unsigned same size
    static_assert(!fitsInType<int8_t>(std::numeric_limits<uint8_t>::max()));
    static_assert(!fitsInType<int16_t>(std::numeric_limits<uint16_t>::max()));
    static_assert(!fitsInType<int32_t>(std::numeric_limits<uint32_t>::max()));
    static_assert(!fitsInType<int64_t>(std::numeric_limits<uint64_t>::max()));

    // signed unsigned different size
    static_assert(!fitsInType<int8_t>(std::numeric_limits<uint16_t>::max()));
    static_assert(!fitsInType<int16_t>(std::numeric_limits<uint32_t>::max()));
    static_assert(!fitsInType<int32_t>(std::numeric_limits<uint64_t>::max()));

    // floating point checks
    static_assert(fitsInType<double>(std::numeric_limits<uint64_t>::max()));
    static_assert(!fitsInType<uint64_t>(std::numeric_limits<double>::max()));

    static_assert(fitsInType<uint8_t>(255.0));
    static_assert(!fitsInType<uint8_t>(255.1));

    EXPECT_FALSE(fitsInType<uint64_t>(std::nextafter(std::numeric_limits<uint64_t>::max(), maxDouble)));
}
}
