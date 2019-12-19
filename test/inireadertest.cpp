#include "infra/inireader.h"

#include <doctest/doctest.h>
#include <fstream>

namespace inf::test {

using namespace doctest;

class IniReaderTest
{
public:
    IniReaderTest()
    : reader(TEST_DATA_DIR "/test.ini")
    {
    }

    IniReader reader;
};

TEST_CASE_FIXTURE(IniReaderTest, "get_int32")
{
    CHECK(reader.get_int32("protocol", "version") == 6);
    CHECK(reader.get_int32("protocol", "VERSION") == 6);
    CHECK(reader.get_int32("PROTOCOL", "VERsiON") == 6);
    CHECK_FALSE(reader.get_int32("PROTOCOL", "value").has_value());

    CHECK_FALSE(reader.get_int32("types", "doesnotexist").has_value());
    CHECK_FALSE(reader.get_int32("doesnotexist", "doesnotexist").has_value());
}

TEST_CASE_FIXTURE(IniReaderTest, "get_string")
{
    CHECK("Bob Smith" == reader.get_string("user", "name"));
    CHECK("bob@smith.com" == reader.get_string("user", "email"));
    CHECK("true" == reader.get_string("user", "active"));
}

TEST_CASE_FIXTURE(IniReaderTest, "get_bool")
{
    CHECK(true == reader.get_bool("user", "active"));
    CHECK(false == reader.get_bool("user", "inactive"));

    CHECK(true == reader.get_bool("types", "onbool"));
    CHECK(false == reader.get_bool("types", "offbool"));

    CHECK(true == reader.get_bool("types", "yesbool"));
    CHECK(false == reader.get_bool("types", "nobool"));

    CHECK(true == reader.get_bool("types", "onebool"));
    CHECK(false == reader.get_bool("types", "zerobool"));

    CHECK(false == reader.get_bool("types", "doesnotexist").has_value());
    CHECK(false == reader.get_bool("doesnotexist", "doesnotexist").has_value());
    CHECK(false == reader.get_bool("user", "name").has_value());
    CHECK(false == reader.get_bool("types", "integer").has_value());
}

TEST_CASE_FIXTURE(IniReaderTest, "get_double")
{
    CHECK(reader.get_double("types", "pi").value() == Approx(3.14159));

    CHECK_FALSE(reader.get_double("types", "doesnotexist").has_value());
    CHECK_FALSE(reader.get_double("doesnotexist", "doesnotexist").has_value());
    CHECK_FALSE(reader.get_double("user", "name").has_value());
}
}
