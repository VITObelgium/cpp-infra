#include "infra/charset.h"

#include <doctest/doctest.h>

namespace inf::test {

TEST_CASE("CharacterSetTest.detectFileEncoding")
{
    CHECK(CharacterSet::Utf8 == detect_character_set(TEST_DATA_DIR "/point_sources_utf8.csv"));
    CHECK(CharacterSet::Utf8 == detect_character_set(TEST_DATA_DIR "/point_sources_utf8_bom.csv"));
    CHECK(CharacterSet::Utf16LE == detect_character_set(TEST_DATA_DIR "/point_sources_utf16LE.csv"));
    CHECK(CharacterSet::Utf16BE == detect_character_set(TEST_DATA_DIR "/point_sources_utf16BE.csv"));
    CHECK(CharacterSet::ISO_8859_1 == detect_character_set(TEST_DATA_DIR "/point_sources_plain_text.csv"));
    CHECK(CharacterSet::ISO_8859_1 == detect_character_set(TEST_DATA_DIR "/point_sources_western.csv"));
}

}
