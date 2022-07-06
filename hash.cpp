#include "infra/hash.h"
#include "infra/string.h"

#ifdef INFRA_HASH_SUPPORT
#include <cryptopp/cryptlib.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md5.h>
#endif

#include <array>
#include <cassert>
#include <sstream>

namespace inf::hash {

using namespace CryptoPP;

#ifdef INFRA_HASH_SUPPORT
std::string hex_encode(std::span<const uint8_t> data)
{
    std::string hexString;
    ArraySource src(data.data(), data.size(), true, new HexEncoder(new StringSink(hexString), false /*lowercase*/));
    return hexString;
}

std::vector<uint8_t> hex_decode(std::string_view hexStringData)
{
    std::vector<uint8_t> data;
    StringSource ss(reinterpret_cast<const uint8_t*>(hexStringData.data()), hexStringData.size(), true, new HexDecoder(new VectorSink(data)));
    return data;
}

std::array<uint8_t, 16> md5(std::span<const uint8_t> data)
{
    std::array<uint8_t, 16> digest;

    Weak::MD5 hash;
    assert(digest.size() == hash.DigestSize());
    auto sink = std::make_unique<HashFilter>(hash, new ArraySink(digest.data(), digest.size()));
    ArraySource src(data.data(), data.size(), true /*pump all*/, sink.release());

    return digest;
}

std::array<uint8_t, 16> md5(std::string_view stringData)
{
    return md5(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(stringData.data()), stringData.size()));
}

std::array<uint8_t, 16> md5(const fs::path& filePath)
{
    std::array<uint8_t, 16> digest;

    Weak::MD5 hash;
    auto sink = std::make_unique<HashFilter>(hash, new ArraySink(digest.data(), digest.size()));
    FileSource src(str::from_u8(filePath.u8string()).c_str(), true /*pump all*/, sink.release());

    return digest;
}

std::array<uint8_t, 64> sha512(std::span<const uint8_t> data)
{
    std::array<uint8_t, 64> digest;

    SHA512 hash;
    assert(digest.size() == hash.DigestSize());
    auto sink = std::make_unique<HashFilter>(hash, new ArraySink(digest.data(), digest.size()));
    ArraySource src(data.data(), data.size(), true /*pump all*/, sink.release());

    return digest;
}

std::array<uint8_t, 64> sha512(const fs::path& filePath)
{
    std::array<uint8_t, 64> digest;

    SHA512 hash;
    assert(digest.size() == hash.DigestSize());
    auto sink = std::make_unique<HashFilter>(hash, new ArraySink(digest.data(), digest.size()));
    FileSource src(str::from_u8(filePath.u8string()).c_str(), true /*pump all*/, sink.release());

    return digest;
}

#endif

std::string md5_string(const fs::path& filePath)
{
    return hex_encode(md5(filePath));
}

std::string md5_string(std::string_view stringData)
{
    return hex_encode(md5(stringData));
}

std::string md5_string(std::span<const uint8_t> data)
{
    return hex_encode(md5(data));
}

std::string sha512_string(const fs::path& filePath)
{
    return hex_encode(sha512(filePath));
}

std::string sha512_string(std::span<const uint8_t> data)
{
    return hex_encode(sha512(data));
}

}
