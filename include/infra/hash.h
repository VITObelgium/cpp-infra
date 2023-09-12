#pragma once

#include "infra/filesystem.h"
#include "infra/span.h"
#include <functional>

namespace inf {

template <typename T, typename... Rest>
void hash_combine(std::size_t& seed, const T& v, const Rest&... rest)
{
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hash_combine(seed, rest), ...);
}

struct StringHash
{
    using is_transparent = void;
    [[nodiscard]] size_t operator()(const char* txt) const
    {
        return std::hash<std::string_view>()(txt);
    }

    [[nodiscard]] size_t operator()(std::string_view txt) const
    {
        return std::hash<std::string_view>()(txt);
    }

    [[nodiscard]] size_t operator()(const std::string& txt) const
    {
        return std::hash<std::string>()(txt);
    }
};

namespace hash {

std::string hex_encode(std::span<const uint8_t> data);
std::vector<uint8_t> hex_decode(std::string_view data);

std::string base64_encode(std::span<const uint8_t> data);
std::vector<uint8_t> base64_decode(std::string_view data);

std::array<uint8_t, 16> md5(std::span<const uint8_t> data);
std::array<uint8_t, 16> md5(std::string_view stringData);
std::array<uint8_t, 16> md5(const fs::path& filePath);
std::string md5_string(const fs::path& filePath);
std::string md5_string(std::string_view stringData);
std::string md5_string(std::span<const uint8_t> data);

std::array<uint8_t, 64> sha512(std::span<const uint8_t> data);
std::array<uint8_t, 64> sha512(const fs::path& filePath);
std::string sha512_string(const fs::path& filePath);
std::string sha512_string(std::span<const uint8_t> data);

}
}
