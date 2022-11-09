#include "infra/csvreader.h"

#include <doctest/doctest.h>
#include <fstream>

namespace inf::test {

using namespace doctest;

static void verifyCsvData(const fs::path& p)
{
    CsvReader reader(p);
    CHECK(reader.column_count() == 33);

    int index = 1;
    for (auto row : reader) {
        REQUIRE(index <= 2);

        if (index == 1) {
            CHECK(row.get_int32(0) == 2015);
            CHECK(row.get_column_as<std::string>(12).value() == "4-nonylphénol");
        } else {
            CHECK(row.get_int32(0) == 2016);
            CHECK(row.get_column_as<std::string>(12).value() == "Anthracène");
        }

        ++index;
    }

    CHECK(index == 3);
}

TEST_CASE("CsvReaderTest.readColumnInfo")
{
    CsvReader reader(TEST_DATA_DIR "/point_sources_utf8.csv");
    CHECK(reader.column_count() == 33);
    CHECK("Sampling location JV Year Date DTS" == reader.column_name(0));
    CHECK("Sampling location NACE Sector EIW" == reader.column_name(1));
    CHECK("Sampling location NACE Sector EIW Name" == reader.column_name(2));
    CHECK("Sampling location NACE Sector JV" == reader.column_name(3));
    CHECK("Sampling location NACE Sector JV Name" == reader.column_name(4));
    CHECK("Sampling location NACE Sub-Sector EIW" == reader.column_name(5));
    CHECK("Sampling location NACE Sub-Sector EIW Name" == reader.column_name(6));
    CHECK("Facility Name" == reader.column_name(7));
    CHECK("Sampling location Number" == reader.column_name(8));
    CHECK("Sampling location Type Code" == reader.column_name(9));
    CHECK("Sampling location Outlet point Effluent X Coordinate" == reader.column_name(10));
    CHECK("Sampling location Outlet point Effluent Y Coordinate" == reader.column_name(11));
    CHECK("Par Description" == reader.column_name(12));
    CHECK("Std Load Unit Symbol MAW" == reader.column_name(13));
    CHECK("Sampling location JV Gross Load OG DTS" == reader.column_name(14));
    CHECK("Facility ID" == reader.column_name(15));
    CHECK("Facility CBB Number" == reader.column_name(16));
    CHECK("Facility Street Name" == reader.column_name(17));
    CHECK("Facility House Number" == reader.column_name(18));
    CHECK("Facility Postal Code" == reader.column_name(19));
    CHECK("Facility Municipality" == reader.column_name(20));
    CHECK("Facility Municipality Code" == reader.column_name(21));
    CHECK("Facility Lambert X Coordinate" == reader.column_name(22));
    CHECK("Facility Lambert Y Coordinate" == reader.column_name(23));
    CHECK("Sampling location Lambert X Coordinate" == reader.column_name(24));
    CHECK("Sampling location Lambert Y Coordinate" == reader.column_name(25));
    CHECK("Par ID" == reader.column_name(26));
    CHECK("Par Symbol" == reader.column_name(27));
    CHECK("Exceeding EPRTR" == reader.column_name(28));
    CHECK("EPRTR facility" == reader.column_name(29));
    CHECK("Reported by" == reader.column_name(30));
    CHECK("Typology" == reader.column_name(31));
    CHECK("Rel uncertainty" == reader.column_name(32));
}

TEST_CASE("CsvReaderTest.readCsvUtf8")
{
    verifyCsvData(TEST_DATA_DIR "/point_sources_utf8.csv");
    verifyCsvData(TEST_DATA_DIR "/point_sources_utf8_bom.csv");
}

TEST_CASE("CsvReaderTest.readCsvWesternEncoding")
{
    verifyCsvData(TEST_DATA_DIR "/point_sources_western.csv");
}

TEST_CASE("CsvReaderTest.readCsvEmptyFields")
{
    auto filePath = fs::temp_directory_path() / "test.csv";

    std::string csv = "int,double,string\n5,20.0,value\n,,";
    file::write_as_text(filePath, csv);

    {
        CsvReader reader(filePath);

        auto iter = reader.begin();
        CHECK((*iter).get_int32(0).value() == 5);
        CHECK((*iter).get_double(1).value() == Approx(20.0));
        CHECK((*iter).get_string(2) == "value");

        ++iter;
        CHECK_FALSE((*iter).get_int32(0).has_value());
        CHECK_FALSE((*iter).get_double(1).has_value());
        CHECK((*iter).get_string(2).empty());
    }

    fs::remove(filePath);
}
}
