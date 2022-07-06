#pragma once

#include "infra/span.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace inf::crypto {

std::vector<uint8_t> encrypt(std::string_view stringData, std::string_view key);
std::vector<uint8_t> encrypt(std::span<const uint8_t> data, std::string_view key);

std::vector<uint8_t> decrypt(std::span<const uint8_t> data, std::string_view key);
std::string decrypt_to_string(std::span<const uint8_t> data, std::string_view key);

}
