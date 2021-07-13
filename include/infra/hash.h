#pragma once

#include "infra/span.h"
#include "infra/filesystem.h"
#include <functional>

namespace inf {

template <typename T, typename... Rest>
void hash_combine(std::size_t& seed, const T& v, const Rest&... rest)
{
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hash_combine(seed, rest), ...);
}

namespace hash {

std::array<uint8_t, 16> md5(std::span<const uint8_t> data);
std::array<uint8_t, 16> md5(const fs::path& filePath);
std::string md5_string(const fs::path& filePath);
std::string md5_string(std::span<const uint8_t> data);

std::array<uint8_t, 64> sha512(std::span<const uint8_t> data);
std::array<uint8_t, 64> sha512(const fs::path& filePath);
std::string sha512_string(const fs::path& filePath);
std::string sha512_string(std::span<const uint8_t> data);


}
}
