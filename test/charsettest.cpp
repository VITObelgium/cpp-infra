#include "infra/charset.h"

#include <gtest/gtest.h>

namespace inf::test {

using namespace testing;

TEST(CharacterSetTest, detectFileEncoding)
{
    EXPECT_EQ(CharacterSet::Utf8, detect_character_set(TEST_DATA_DIR "/point_sources_utf8.csv"));
    EXPECT_EQ(CharacterSet::Utf8, detect_character_set(TEST_DATA_DIR "/point_sources_utf8_bom.csv"));
    EXPECT_EQ(CharacterSet::Utf16LE, detect_character_set(TEST_DATA_DIR "/point_sources_utf16LE.csv"));
    EXPECT_EQ(CharacterSet::Utf16BE, detect_character_set(TEST_DATA_DIR "/point_sources_utf16BE.csv"));
    EXPECT_EQ(CharacterSet::ISO_8859_1, detect_character_set(TEST_DATA_DIR "/point_sources_plain_text.csv"));
    EXPECT_EQ(CharacterSet::ISO_8859_1, detect_character_set(TEST_DATA_DIR "/point_sources_western.csv"));
}

}
