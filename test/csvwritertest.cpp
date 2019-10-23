#include "infra/csvwriter.h"
#include "infra/csvreader.h"

#include <fstream>
#include <gtest/gtest.h>

namespace inf::test {

using namespace std::string_literals;
using namespace testing;

template <char Sep>
struct digit_separator : std::numpunct<char>
{
    char do_decimal_point() const
    {
        return Sep;
    }
};

TEST(CsvWriterTest, writeCsvWithHeader)
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

    CsvReader csv(csvPath);
    ASSERT_EQ(3, csv.column_count());
    EXPECT_EQ("col1", csv.column_name(0));
    EXPECT_EQ("col 2", csv.column_name(1));
    EXPECT_EQ("col;3", csv.column_name(2));

    auto row = csv.begin();
    EXPECT_EQ("value 1", row->get_string(0));
    EXPECT_EQ("value 2", row->get_string(1));
    EXPECT_EQ("value;3", row->get_string(2));

    ++row;
    EXPECT_EQ(123, row->get_int32(0));
    EXPECT_EQ("123.12", row->get_string(1));
    EXPECT_EQ("nan", row->get_string(2));

    fs::remove(csvPath);
}

TEST(CsvWriterTest, writeCsvSeparatorIsDigitSeparator)
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

    CsvReader csv(csvPath);
    ASSERT_EQ(3, csv.column_count());

    auto row = csv.begin();
    EXPECT_EQ("123,1235", row->get_string(0));
    EXPECT_EQ("-123,1200", row->get_string(1));
    EXPECT_EQ("0,0000", row->get_string(2));

    fs::remove(csvPath);
}
}