#include "infra/inireader.h"

#include <fstream>
#include <gtest/gtest.h>

namespace infra::test {

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

TEST_F(IniReaderTest, getInteger)
{
    EXPECT_EQ(6, reader.getInteger("protocol", "version"));
    EXPECT_EQ(6, reader.getInteger("protocol", "VERSION"));
    EXPECT_EQ(6, reader.getInteger("PROTOCOL", "VERsiON"));
    EXPECT_FALSE(reader.getInteger("PROTOCOL", "value").has_value());

    EXPECT_FALSE(reader.getInteger("types", "doesnotexist").has_value());
    EXPECT_FALSE(reader.getInteger("doesnotexist", "doesnotexist").has_value());
}

TEST_F(IniReaderTest, getString)
{
    EXPECT_EQ("Bob Smith", reader.getString("user", "name"));
    EXPECT_EQ("bob@smith.com", reader.getString("user", "email"));
    EXPECT_EQ("true", reader.getString("user", "active"));
}

TEST_F(IniReaderTest, getBool)
{
    EXPECT_EQ(true, reader.getBool("user", "active"));
    EXPECT_EQ(false, reader.getBool("user", "inactive"));

    EXPECT_EQ(true, reader.getBool("types", "onbool"));
    EXPECT_EQ(false, reader.getBool("types", "offbool"));

    EXPECT_EQ(true, reader.getBool("types", "yesbool"));
    EXPECT_EQ(false, reader.getBool("types", "nobool"));

    EXPECT_EQ(true, reader.getBool("types", "onebool"));
    EXPECT_EQ(false, reader.getBool("types", "zerobool"));

    EXPECT_FALSE(reader.getBool("types", "doesnotexist").has_value());
    EXPECT_FALSE(reader.getBool("doesnotexist", "doesnotexist").has_value());
    EXPECT_FALSE(reader.getBool("user", "name").has_value());
    EXPECT_FALSE(reader.getBool("types", "integer").has_value());
}

TEST_F(IniReaderTest, getReal)
{
    EXPECT_DOUBLE_EQ(3.14159, reader.getReal("types", "pi").value());

    EXPECT_FALSE(reader.getReal("types", "doesnotexist").has_value());
    EXPECT_FALSE(reader.getReal("doesnotexist", "doesnotexist").has_value());
    EXPECT_FALSE(reader.getReal("user", "name").has_value());
}
}
