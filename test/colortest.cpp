#include "infra/color.h"
#include "infra/exception.h"
#include "infra/test/printsupport.h"

#include <doctest/doctest.h>

namespace inf::test {

TEST_CASE("ColorTest.from_hex_rgb")
{
    CHECK(Color("#000000") == Color(0, 0, 0, 255));
    CHECK(Color("#FFFFFF") == Color(255, 255, 255, 255));
    CHECK(Color("#FF0000") == Color(255, 0, 0, 255));
    CHECK(Color("#00FF00") == Color(0, 255, 0, 255));
    CHECK(Color("#0000FF") == Color(0, 0, 255, 255));

    CHECK(Color("#000000") == Color(0, 0, 0, 255));
    CHECK(Color("#fffFFF") == Color(255, 255, 255, 255));
    CHECK(Color("#ff0000") == Color(255, 0, 0, 255));
    CHECK(Color("#00fF00") == Color(0, 255, 0, 255));
    CHECK(Color("#0000FF") == Color(0, 0, 255, 255));

    CHECK(Color("#19E624") == Color(25, 230, 36, 255));
}

TEST_CASE("ColorTest.from_hex_argb")
{
    CHECK(Color("#ff000000") == Color(0, 0, 0, 255));
    CHECK(Color("#ffffFFFF") == Color(255, 255, 255, 255));
    CHECK(Color("#ffff0000") == Color(255, 0, 0, 255));
    CHECK(Color("#ff00FF00") == Color(0, 255, 0, 255));
    CHECK(Color("#ff0000FF") == Color(0, 0, 255, 255));

    CHECK(Color("#00000000") == Color(0, 0, 0, 0));
    CHECK(Color("#64ffFFFF") == Color(255, 255, 255, 100));
    CHECK(Color("#96ff0000") == Color(255, 0, 0, 150));
    CHECK(Color("#c800FF00") == Color(0, 255, 0, 200));
    CHECK(Color("#ff0000FF") == Color(0, 0, 255, 255));

    CHECK(Color("#FF19E624") == Color(25, 230, 36, 255));
}

TEST_CASE("ColorTest.from_invalid_hex")
{
    CHECK_THROWS_AS(Color(""), InvalidArgument);
    CHECK_THROWS_AS(Color("#"), InvalidArgument);
    CHECK_THROWS_AS(Color("#########"), InvalidArgument);
    CHECK_THROWS_AS(Color("#0011ZZ"), InvalidArgument);
    CHECK_THROWS_AS(Color("#FFFFFFF"), InvalidArgument);
}

TEST_CASE("ColorTest.to_hex_rgb")
{
    CHECK("#000000" == Color(0, 0, 0).to_hex_rgb());
    CHECK("#FFFFFF" == Color(255, 255, 255).to_hex_rgb());
    CHECK("#FF0000" == Color(255, 0, 0).to_hex_rgb());
    CHECK("#00FF00" == Color(0, 255, 0).to_hex_rgb());
    CHECK("#0000FF" == Color(0, 0, 255).to_hex_rgb());

    CHECK("#000000" == Color(0, 0, 0, 0).to_hex_rgb());
    CHECK("#FFFFFF" == Color(255, 255, 255, 100).to_hex_rgb());
    CHECK("#FF0000" == Color(255, 0, 0, 150).to_hex_rgb());
    CHECK("#00FF00" == Color(0, 255, 0, 200).to_hex_rgb());
    CHECK("#0000FF" == Color(0, 0, 255, 255).to_hex_rgb());

    CHECK("#19E624" == Color(25, 230, 36, 255).to_hex_rgb());
}

TEST_CASE("ColorTest.to_hex_argb")
{
    CHECK("#FF000000" == Color(0, 0, 0).to_hex_argb());
    CHECK("#FFFFFFFF" == Color(255, 255, 255).to_hex_argb());
    CHECK("#FFFF0000" == Color(255, 0, 0).to_hex_argb());
    CHECK("#FF00FF00" == Color(0, 255, 0).to_hex_argb());
    CHECK("#FF0000FF" == Color(0, 0, 255).to_hex_argb());

    CHECK("#00000000" == Color(0, 0, 0, 0).to_hex_argb());
    CHECK("#64FFFFFF" == Color(255, 255, 255, 100).to_hex_argb());
    CHECK("#96FF0000" == Color(255, 0, 0, 150).to_hex_argb());
    CHECK("#C800FF00" == Color(0, 255, 0, 200).to_hex_argb());
    CHECK("#FF0000FF" == Color(0, 0, 255, 255).to_hex_argb());

    CHECK("#FF19E624" == Color(25, 230, 36, 255).to_hex_argb());
}

}
