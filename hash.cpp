#include "infra/hash.h"

#ifdef INFRA_HASH_SUPPORT
#include <cryptopp/cryptlib.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md5.h>
#endif

#include <cassert>
#include <sstream>

namespace inf::hash {

using namespace CryptoPP;

static std::string hash_string(std::span<const uint8_t> hash)
{
    std::string hashString;
    ArraySource src(hash.data(), hash.size(), true, new HexEncoder(new StringSink(hashString), false /*lowercase*/));
    return hashString;
}

#ifdef INFRA_HASH_SUPPORT
std::array<uint8_t, 16> md5(std::span<const uint8_t> data)
{
    std::array<uint8_t, 16> digest;

    Weak::MD5 hash;
    assert(digest.size() == hash.DigestSize());
    auto sink = std::make_unique<HashFilter>(hash, new ArraySink(digest.data(), digest.size()));
    ArraySource src(data.data(), data.size(), true /*pump all*/, sink.release());

    return digest;
}

std::array<uint8_t, 16> md5(const fs::path& filePath)
{
    std::array<uint8_t, 16> digest;

    Weak::MD5 hash;
    auto sink = std::make_unique<HashFilter>(hash, new ArraySink(digest.data(), digest.size()));
    FileSource src(filePath.u8string().c_str(), true /*pump all*/, sink.release());

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
    FileSource src(filePath.u8string().c_str(), true /*pump all*/, sink.release());

    return digest;
}

#endif

std::string md5_string(const fs::path& filePath)
{
    return hash_string(md5(filePath));
}

std::string md5_string(std::span<const uint8_t> data)
{
    return hash_string(md5(data));
}

std::string sha512_string(const fs::path& filePath)
{
    return hash_string(sha512(filePath));
}

std::string sha512_string(std::span<const uint8_t> data)
{
    return hash_string(sha512(data));
}

}
