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
    for (auto row : reader) {
        ASSERT_LE(index, 2);

        if (index == 1) {
            EXPECT_EQ(2015, row.get_int32(0));
            EXPECT_EQ(u8"4-nonylphénol", row.get_column_as<std::string>(12).value());
        } else {
            EXPECT_EQ(2016, row.get_int32(0));
            EXPECT_EQ(u8"Anthracène", row.get_column_as<std::string>(12).value());
        }

        ++index;
    }

    EXPECT_EQ(3, index);
}

TEST(CsvReaderTest, readColumnInfo)
{
    CsvReader reader(TEST_DATA_DIR "/point_sources_utf8.csv");
    EXPECT_EQ(33, reader.column_count());
    EXPECT_EQ("Sampling location JV Year Date DTS", reader.column_name(0));
    EXPECT_EQ("Sampling location NACE Sector EIW", reader.column_name(1));
    EXPECT_EQ("Sampling location NACE Sector EIW Name", reader.column_name(2));
    EXPECT_EQ("Sampling location NACE Sector JV", reader.column_name(3));
    EXPECT_EQ("Sampling location NACE Sector JV Name", reader.column_name(4));
    EXPECT_EQ("Sampling location NACE Sub-Sector EIW", reader.column_name(5));
    EXPECT_EQ("Sampling location NACE Sub-Sector EIW Name", reader.column_name(6));
    EXPECT_EQ("Facility Name", reader.column_name(7));
    EXPECT_EQ("Sampling location Number", reader.column_name(8));
    EXPECT_EQ("Sampling location Type Code", reader.column_name(9));
    EXPECT_EQ("Sampling location Outlet point Effluent X Coordinate", reader.column_name(10));
    EXPECT_EQ("Sampling location Outlet point Effluent Y Coordinate", reader.column_name(11));
    EXPECT_EQ("Par Description", reader.column_name(12));
    EXPECT_EQ("Std Load Unit Symbol MAW", reader.column_name(13));
    EXPECT_EQ("Sampling location JV Gross Load OG DTS", reader.column_name(14));
    EXPECT_EQ("Facility ID", reader.column_name(15));
    EXPECT_EQ("Facility CBB Number", reader.column_name(16));
    EXPECT_EQ("Facility Street Name", reader.column_name(17));
    EXPECT_EQ("Facility House Number", reader.column_name(18));
    EXPECT_EQ("Facility Postal Code", reader.column_name(19));
    EXPECT_EQ("Facility Municipality", reader.column_name(20));
    EXPECT_EQ("Facility Municipality Code", reader.column_name(21));
    EXPECT_EQ("Facility Lambert X Coordinate", reader.column_name(22));
    EXPECT_EQ("Facility Lambert Y Coordinate", reader.column_name(23));
    EXPECT_EQ("Sampling location Lambert X Coordinate", reader.column_name(24));
    EXPECT_EQ("Sampling location Lambert Y Coordinate", reader.column_name(25));
    EXPECT_EQ("Par ID", reader.column_name(26));
    EXPECT_EQ("Par Symbol", reader.column_name(27));
    EXPECT_EQ("Exceeding EPRTR", reader.column_name(28));
    EXPECT_EQ("EPRTR facility", reader.column_name(29));
    EXPECT_EQ("Reported by", reader.column_name(30));
    EXPECT_EQ("Typology", reader.column_name(31));
    EXPECT_EQ("Rel uncertainty", reader.column_name(32));
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

TEST(CsvReaderTest, readCsvEmptyFields)
{
    auto filePath = fs::temp_directory_path() / "test.csv";

    std::string csv = "int,double,string\n5,20.0,value\n,,";
    file::write_as_text(filePath, csv);

    {
        CsvReader reader(filePath);

        auto iter = reader.begin();
        EXPECT_EQ(5, (*iter).get_int32(0).value());
        EXPECT_DOUBLE_EQ(20.0, (*iter).get_double(1).value());
        EXPECT_EQ("value", (*iter).get_string(2));

        ++iter;
        EXPECT_FALSE((*iter).get_int32(0).has_value());
        EXPECT_FALSE((*iter).get_double(1).has_value());
        EXPECT_TRUE((*iter).get_string(2).empty());
    }

    fs::remove(filePath);
}
}
