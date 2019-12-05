#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "infra/color.h"
#include "infra/exception.h"
#include "infra/test/printsupport.h"

namespace inf::test {

using namespace testing;

TEST(ColorTest, from_hex_rgb)
{
    EXPECT_EQ(Color("#000000"), Color(0, 0, 0, 255));
    EXPECT_EQ(Color("#FFFFFF"), Color(255, 255, 255, 255));
    EXPECT_EQ(Color("#FF0000"), Color(255, 0, 0, 255));
    EXPECT_EQ(Color("#00FF00"), Color(0, 255, 0, 255));
    EXPECT_EQ(Color("#0000FF"), Color(0, 0, 255, 255));

    EXPECT_EQ(Color("#000000"), Color(0, 0, 0, 255));
    EXPECT_EQ(Color("#fffFFF"), Color(255, 255, 255, 255));
    EXPECT_EQ(Color("#ff0000"), Color(255, 0, 0, 255));
    EXPECT_EQ(Color("#00fF00"), Color(0, 255, 0, 255));
    EXPECT_EQ(Color("#0000FF"), Color(0, 0, 255, 255));

    EXPECT_EQ(Color("#19E624"), Color(25, 230, 36, 255));
}

TEST(ColorTest, from_hex_argb)
{
    EXPECT_EQ(Color("#ff000000"), Color(0, 0, 0, 255));
    EXPECT_EQ(Color("#ffffFFFF"), Color(255, 255, 255, 255));
    EXPECT_EQ(Color("#ffff0000"), Color(255, 0, 0, 255));
    EXPECT_EQ(Color("#ff00FF00"), Color(0, 255, 0, 255));
    EXPECT_EQ(Color("#ff0000FF"), Color(0, 0, 255, 255));

    EXPECT_EQ(Color("#00000000"), Color(0, 0, 0, 0));
    EXPECT_EQ(Color("#64ffFFFF"), Color(255, 255, 255, 100));
    EXPECT_EQ(Color("#96ff0000"), Color(255, 0, 0, 150));
    EXPECT_EQ(Color("#c800FF00"), Color(0, 255, 0, 200));
    EXPECT_EQ(Color("#ff0000FF"), Color(0, 0, 255, 255));

    EXPECT_EQ(Color("#FF19E624"), Color(25, 230, 36, 255));
}

TEST(ColorTest, from_invalid_hex)
{
    EXPECT_THROW(Color(""), InvalidArgument);
    EXPECT_THROW(Color("#"), InvalidArgument);
    EXPECT_THROW(Color("#########"), InvalidArgument);
    EXPECT_THROW(Color("#0011ZZ"), InvalidArgument);
    EXPECT_THROW(Color("#FFFFFFF"), InvalidArgument);
}

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
