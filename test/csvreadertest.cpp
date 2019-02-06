#include "infra/csvreader.h"

#include <fstream>
#include <gtest/gtest.h>

namespace inf::test {

using namespace testing;

static void verifyCsvData(const fs::path& p)
{
    CsvReader reader(p);
    EXPECT_EQ(33, reader.column_count());

    int index = 1;
    for (auto line : reader) {
        ASSERT_LE(index, 2);

        if (index == 1) {
            EXPECT_EQ(2015, line.get_int32(0));
            EXPECT_EQ(u8"4-nonylphénol", line.get_column_as<std::string>(12).value());
        } else {
            auto val = line.get_int32(0);
            EXPECT_EQ(2016, line.get_int32(0));
            EXPECT_EQ(u8"Anthracène", line.get_column_as<std::string>(12).value());
        }

        ++index;
    }

    EXPECT_EQ(3, index);
}

TEST(CsvReaderTest, readCsvUtf8)
{
    verifyCsvData(TEST_DATA_DIR "/point_sources_utf8.csv");
    verifyCsvData(TEST_DATA_DIR "/point_sources_utf8_bom.csv");
}

TEST(CsvReaderTest, readCsvWesternEncoding)
{
    verifyCsvData(TEST_DATA_DIR "/point_sources_western.csv");
}
}
