#include "infra/hash.h"

#include <doctest/doctest.h>

namespace inf::test {

TEST_CASE("Hash")
{
    const auto filePath = fs::u8path(TEST_DATA_DIR) / "epsg31370.tif";
    
    SUBCASE("MD5 string")
    {
        constexpr std::string_view md5 = "29367e8926f2a126a2e2ab8d13815b69";

        CHECK(hash::md5_string(filePath) == md5);
        CHECK(hash::md5_string(file::read(filePath)) == md5);
    }

    SUBCASE("SHA512 string")
    {
        constexpr std::string_view sha512 = "0ef92fd1dbc82f86b59da2a8571432ac6327a94bac791dc6b5fdb845ad91a42605f9d12f71a26b0dc221622bec42d88dc5667cd9d3e284f97232f16a9d322014";

        CHECK(hash::sha512_string(filePath) == sha512);
        CHECK(hash::sha512_string(file::read(filePath)) == sha512);
    }
}

}
