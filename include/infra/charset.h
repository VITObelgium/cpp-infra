#pragma once

#include "infra/filesystem.h"

namespace inf {

enum class CharacterSet
{
    Utf8,
    Utf16LE,
    Utf16BE,
    ISO_8859_1, // Western
    Unknown,
};

CharacterSet detect_character_set(const fs::path& path);
CharacterSet detect_character_set_from_data(std::string_view data);

std::string convert_to_utf8(std::string_view input);
}
