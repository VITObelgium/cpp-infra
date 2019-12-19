#include "infra/colormap.h"

#include <doctest/doctest.h>

namespace inf::test {

TEST_CASE("ColorMapTest.colorMapFromDict")
{
    ColorMap cm(Cmap::summer);

    CHECK(Color(64, 160, 102) == cm.get_color(0.25f));
    CHECK(Color(128, 192, 102) == cm.get_color(0.5f));
    CHECK(Color(191, 223, 102) == cm.get_color(0.75f));
    CHECK(Color(255, 255, 102) == cm.get_color(1.f));
}

TEST_CASE("ColorMapTest.colorMapFromVector")
{
    ColorMap cm(Cmap::Blues);

    CHECK(Color(247, 251, 255) == cm.get_color(0.f));
    CHECK(Color(198, 219, 239) == cm.get_color(0.25f));
    CHECK(Color(106, 174, 214) == cm.get_color(0.5f));
    CHECK(Color(33, 113, 181) == cm.get_color(0.75f));
    CHECK(Color(8, 48, 107) == cm.get_color(1.f));
}

TEST_CASE("ColorMapTest.colorMapFromMapper")
{
    ColorMap cm(Cmap::rainbow);

    CHECK(Color(128, 0, 255) == cm.get_color(0.f));
    CHECK(Color(0, 181, 235) == cm.get_color(0.25f));
    CHECK(Color(255, 0, 0) == cm.get_color(1.f));
}

}
