#include "infra/geocoder.h"
#include "infra/coordinate.h"
#include "infra/test/printsupport.h"

#include <cpl_http.h>
#include <doctest/doctest.h>
#include <fstream>

namespace inf::test {

using namespace doctest;
using namespace std::string_literals;

TEST_CASE("Geocoder.geocode_single")
{
    if (!CPLHTTPEnabled()) {
        return;
    }

    Geocoder::Options options;
    options.readCache  = false;
    options.writeCache = false;

    Geocoder gc(options);
    gc.allow_unsafe_ssl(); // Coorporate proxy issues

    SUBCASE("valid result boeretang")
    {
        auto coord = gc.geocode_single("Boeretang 200 Mol");
        REQUIRE(coord.has_value());
        CHECK(coord->latitude == Approx(51.219066).epsilon(0.01));
        CHECK(coord->longitude == Approx(5.093644).epsilon(0.01));
    }

    SUBCASE("valid result lommel")
    {
        auto coord = gc.geocode_single("Kloosterstraat 44 Lommel");
        REQUIRE(coord.has_value());
        CHECK(coord->latitude == Approx(51.2299701).epsilon(0.01));
        CHECK(coord->longitude == Approx(5.31576194045124).epsilon(0.01));
    }

    SUBCASE("valid result lommel no street number")
    {
        auto coord = gc.geocode_single("Kloosterstraat Lommel");
        REQUIRE(coord.has_value());
        // High epsilon, because of missing street number
        CHECK(coord->latitude == Approx(51.2314736).epsilon(0.1));
        CHECK(coord->longitude == Approx(5.3246890716).epsilon(0.1));
    }

    SUBCASE("valid result with country restriction")
    {
        auto coord = gc.geocode_single("Boeretang 200 Mol", "be");
        REQUIRE(coord.has_value());
        CHECK(coord->latitude == Approx(51.219066).epsilon(0.01));
        CHECK(coord->longitude == Approx(5.093644).epsilon(0.01));
    }

    SUBCASE("no match due to country restriction")
    {
        auto coord = gc.geocode_single("Boeretang 200 Mol", "de");
        CHECK_FALSE(coord.has_value());
    }
}

}
