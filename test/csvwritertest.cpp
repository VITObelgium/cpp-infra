#include "infra/csvwriter.h"
#include "infra/csvreader.h"

#include <doctest/doctest.h>
#include <fstream>

namespace inf::test {

using namespace std::string_literals;

template <char Sep>
struct digit_separator : std::numpunct<char>
{
    char do_decimal_point() const
    {
        return Sep;
    }
};

TEST_CASE("CsvWriterTest.writeCsvWithHeader")
{
    auto csvPath = fs::temp_directory_path() / "test.csv";

    CsvWriter::Settings settings;
    settings.separator = ';';
    settings.precision = 2;

    {
        CsvWriter csv(csvPath, settings);
        csv.write_header(std::vector<std::string>{"col1"s, "col 2"s, "col;3"s});
        csv.write_line(std::vector<std::string>{"value 1"s, "value 2"s, "value;3"s});
        csv.write_column_value(123);
        csv.write_column_value(123.123456789);
        csv.write_column_value(std::numeric_limits<double>::quiet_NaN());
        csv.new_line();
    }

    {
        CsvReader csv(csvPath);
        REQUIRE(csv.column_count() == 3);
        CHECK("col1" == csv.column_name(0));
        CHECK("col 2" == csv.column_name(1));
        CHECK("col;3" == csv.column_name(2));

        auto row = csv.begin();
        CHECK("value 1" == row->get_string(0));
        CHECK("value 2" == row->get_string(1));
        CHECK("value;3" == row->get_string(2));

        ++row;
        CHECK(123 == row->get_int32(0));
        CHECK("123.12" == row->get_string(1));
        CHECK("nan" == row->get_string(2));
    }

    fs::remove(csvPath);
}

TEST_CASE("CsvWriterTest.writeCsvSeparatorIsDigitSeparator")
{
    auto csvPath = fs::temp_directory_path() / "test.csv";

    CsvWriter::Settings settings;
    // this is also the floating point digit separator
    // so quoting of floating point values is required
    settings.separator = ',';
    settings.precision = 4;
    settings.locale    = std::locale(std::locale::classic(), new digit_separator<','>);

    {
        CsvWriter csv(csvPath, settings);
        csv.write_header(std::vector<std::string>{"col1"s, "col2"s, "col3"s});
        csv.write_column_value(123.123456789);
        csv.write_column_value(-123.12);
        csv.write_column_value(0.0);
        csv.new_line();
    }

    {
        CsvReader csv(csvPath);
        REQUIRE(csv.column_count() == 3);

        auto row = csv.begin();
        CHECK("123,1235" == row->get_string(0));
        CHECK("-123,1200" == row->get_string(1));
        CHECK("0,0000" == row->get_string(2));
    }

    fs::remove(csvPath);
}
}
