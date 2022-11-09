#include "infra/crypto.h"
#include "infra/hash.h"

#include <doctest/doctest.h>

namespace inf::test {

static std::string data_as_string(std::span<const uint8_t> data)
{
    return std::string(reinterpret_cast<const char*>(data.data()), data.size());
}

TEST_CASE("Crypto")
{
    const std::string encryptionKey1 = "___1234567890___";
    const std::string encryptionKey2 = "___1234567890__.";

    SUBCASE("encrypt-decrypt data")
    {
        const std::vector<uint8_t> myBiggestSecret = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};

        auto encrypted = crypto::encrypt(myBiggestSecret, encryptionKey1);
        auto decrypted = crypto::decrypt(encrypted, encryptionKey1);

        CHECK(myBiggestSecret == decrypted);
        CHECK(myBiggestSecret != encrypted);
    }

    SUBCASE("encrypt-decrypt string")
    {
        const std::string myBiggestSecret = "This is my biggest secret ever";

        auto encrypted = hash::hex_encode(crypto::encrypt(myBiggestSecret, encryptionKey1));
        auto decrypted = crypto::decrypt_to_string(hash::hex_decode(encrypted), encryptionKey1);

        CHECK(myBiggestSecret == decrypted);
        CHECK(myBiggestSecret != encrypted);
    }
}

}
