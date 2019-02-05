#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "infra/colormap.h"

namespace inf {
void PrintTo(const Color& color, std::ostream* os)
{
    *os << fmt::format("{}", color);
}
}

namespace inf::test {

using namespace testing;

TEST(ColorMapTest, colorMapFromDict)
{
    ColorMap cm(Cmap::summer);

    EXPECT_EQ(Color(64, 160, 102), cm.get_color(0.25f));
    EXPECT_EQ(Color(128, 192, 102), cm.get_color(0.5f));
    EXPECT_EQ(Color(191, 223, 102), cm.get_color(0.75f));
    EXPECT_EQ(Color(255, 255, 102), cm.get_color(1.f));
}

TEST(ColorMapTest, colorMapFromVector)
{
    ColorMap cm(Cmap::Blues);

    EXPECT_EQ(Color(247, 251, 255), cm.get_color(0.f));
    EXPECT_EQ(Color(198, 219, 239), cm.get_color(0.25f));
    EXPECT_EQ(Color(106, 174, 214), cm.get_color(0.5f));
    EXPECT_EQ(Color(33, 113, 181), cm.get_color(0.75f));
    EXPECT_EQ(Color(8, 48, 107), cm.get_color(1.f));
}

TEST(ColorMapTest, colorMapFromMapper)
{
    ColorMap cm(Cmap::rainbow);

    EXPECT_EQ(Color(128, 0, 255), cm.get_color(0.f));
    EXPECT_EQ(Color(0, 181, 235), cm.get_color(0.25f));
    EXPECT_EQ(Color(255, 0, 0), cm.get_color(1.f));
}

}
