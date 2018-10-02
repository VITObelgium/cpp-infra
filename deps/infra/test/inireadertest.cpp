#include "infra/inireader.h"

#include <fstream>
#include <gtest/gtest.h>

namespace inf::test {

using namespace testing;

class IniReaderTest : public Test
{
public:
    IniReaderTest()
    : reader(TEST_DATA_DIR "/test.ini")
    {
    }

    IniReader reader;
};

TEST_F(IniReaderTest, get_int32)
{
    EXPECT_EQ(6, reader.get_int32("protocol", "version"));
    EXPECT_EQ(6, reader.get_int32("protocol", "VERSION"));
    EXPECT_EQ(6, reader.get_int32("PROTOCOL", "VERsiON"));
    EXPECT_FALSE(reader.get_int32("PROTOCOL", "value").has_value());

    EXPECT_FALSE(reader.get_int32("types", "doesnotexist").has_value());
    EXPECT_FALSE(reader.get_int32("doesnotexist", "doesnotexist").has_value());
}

TEST_F(IniReaderTest, get_string)
{
    EXPECT_EQ("Bob Smith", reader.get_string("user", "name"));
    EXPECT_EQ("bob@smith.com", reader.get_string("user", "email"));
    EXPECT_EQ("true", reader.get_string("user", "active"));
}

TEST_F(IniReaderTest, get_bool)
{
    EXPECT_EQ(true, reader.get_bool("user", "active"));
    EXPECT_EQ(false, reader.get_bool("user", "inactive"));

    EXPECT_EQ(true, reader.get_bool("types", "onbool"));
    EXPECT_EQ(false, reader.get_bool("types", "offbool"));

    EXPECT_EQ(true, reader.get_bool("types", "yesbool"));
    EXPECT_EQ(false, reader.get_bool("types", "nobool"));

    EXPECT_EQ(true, reader.get_bool("types", "onebool"));
    EXPECT_EQ(false, reader.get_bool("types", "zerobool"));

    EXPECT_FALSE(reader.get_bool("types", "doesnotexist").has_value());
    EXPECT_FALSE(reader.get_bool("doesnotexist", "doesnotexist").has_value());
    EXPECT_FALSE(reader.get_bool("user", "name").has_value());
    EXPECT_FALSE(reader.get_bool("types", "integer").has_value());
}

TEST_F(IniReaderTest, get_double)
{
    EXPECT_DOUBLE_EQ(3.14159, reader.get_double("types", "pi").value());

    EXPECT_FALSE(reader.get_double("types", "doesnotexist").has_value());
    EXPECT_FALSE(reader.get_double("doesnotexist", "doesnotexist").has_value());
    EXPECT_FALSE(reader.get_double("user", "name").has_value());
}
}
