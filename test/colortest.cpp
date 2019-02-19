#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "infra/color.h"

namespace inf::test {

using namespace testing;

TEST(ColorTest, to_hex_rgb)
{
    EXPECT_EQ("#000000", Color(0, 0, 0).to_hex_rgb());
    EXPECT_EQ("#FFFFFF", Color(255, 255, 255).to_hex_rgb());
    EXPECT_EQ("#FF0000", Color(255, 0, 0).to_hex_rgb());
    EXPECT_EQ("#00FF00", Color(0, 255, 0).to_hex_rgb());
    EXPECT_EQ("#0000FF", Color(0, 0, 255).to_hex_rgb());

    EXPECT_EQ("#000000", Color(0, 0, 0, 0).to_hex_rgb());
    EXPECT_EQ("#FFFFFF", Color(255, 255, 255, 100).to_hex_rgb());
    EXPECT_EQ("#FF0000", Color(255, 0, 0, 150).to_hex_rgb());
    EXPECT_EQ("#00FF00", Color(0, 255, 0, 200).to_hex_rgb());
    EXPECT_EQ("#0000FF", Color(0, 0, 255, 255).to_hex_rgb());

    EXPECT_EQ("#19E624", Color(25, 230, 36, 255).to_hex_rgb());
}

TEST(ColorTest, to_hex_argb)
{
    EXPECT_EQ("#FF000000", Color(0, 0, 0).to_hex_argb());
    EXPECT_EQ("#FFFFFFFF", Color(255, 255, 255).to_hex_argb());
    EXPECT_EQ("#FFFF0000", Color(255, 0, 0).to_hex_argb());
    EXPECT_EQ("#FF00FF00", Color(0, 255, 0).to_hex_argb());
    EXPECT_EQ("#FF0000FF", Color(0, 0, 255).to_hex_argb());

    EXPECT_EQ("#00000000", Color(0, 0, 0, 0).to_hex_argb());
    EXPECT_EQ("#64FFFFFF", Color(255, 255, 255, 100).to_hex_argb());
    EXPECT_EQ("#96FF0000", Color(255, 0, 0, 150).to_hex_argb());
    EXPECT_EQ("#C800FF00", Color(0, 255, 0, 200).to_hex_argb());
    EXPECT_EQ("#FF0000FF", Color(0, 0, 255, 255).to_hex_argb());

    EXPECT_EQ("#FF19E624", Color(25, 230, 36, 255).to_hex_argb());
}

}
