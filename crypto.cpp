#include "infra/crypto.h"
#include "infra/exception.h"
#include "infra/hash.h"
#include "infra/enumutils.h"

#include <array>
#include <cryptopp/blowfish.h>
#include <cryptopp/hex.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>

namespace inf::crypto {

using namespace CryptoPP;

void throw_on_invalid_key_length(std::string_view key)
{
    if (key.size() < Blowfish::MIN_KEYLENGTH || key.size() > Blowfish::MAX_KEYLENGTH) {
        throw RuntimeError("Invalid encryption key size, lengt must be in range [{}-{}]", enum_value(Blowfish::MIN_KEYLENGTH), enum_value(Blowfish::MAX_KEYLENGTH));
    }
}

std::vector<uint8_t> encrypt(std::string_view stringData, std::string_view key)
{
    return encrypt(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(stringData.data()), stringData.size()), key);
}

std::vector<uint8_t> encrypt(std::span<const uint8_t> data, std::string_view key)
{
    throw_on_invalid_key_length(key);

    try {
        std::vector<uint8_t> encrypted;
        AutoSeededRandomPool prng;

        std::array<byte, Blowfish::BLOCKSIZE> iv;
        iv.fill(0);

        CBC_Mode<Blowfish>::Encryption e;
        e.SetKeyWithIV(reinterpret_cast<const uint8_t*>(key.data()), key.size(), iv.data(), iv.size());

        // The StreamTransformationFilter adds padding as required.
        // ECB and CBC Mode must be padded to the block size of the cipher.
        ArraySource ss(data.data(), data.size(), true, new StreamTransformationFilter(e, new VectorSink(encrypted)));
        return encrypted;
    } catch (const CryptoPP::Exception& e) {
        throw RuntimeError("Failed to encrypt data: {}", e.what());
    }
}

std::vector<uint8_t> decrypt(std::span<const uint8_t> data, std::string_view key)
{
    throw_on_invalid_key_length(key);

    try {
        std::vector<uint8_t> decrypted;
        AutoSeededRandomPool prng;

        std::array<byte, Blowfish::BLOCKSIZE> iv;
        iv.fill(0);

        CBC_Mode<Blowfish>::Decryption d;
        d.SetKeyWithIV(reinterpret_cast<const uint8_t*>(key.data()), key.size(), iv.data(), iv.size());

        // The StreamTransformationFilter removes padding as required.
        ArraySource ss(data.data(), data.size(), true, new StreamTransformationFilter(d, new VectorSink(decrypted)));
        return decrypted;
    } catch (const CryptoPP::Exception& e) {
        throw RuntimeError("Failed to decrypt data {}", e.what());
    }
}

std::string decrypt_to_string(std::span<const uint8_t> data, std::string_view key)
{
    const auto decrypted = decrypt(data, key);
    return std::string(reinterpret_cast<const char*>(decrypted.data()), decrypted.size());
}

}
