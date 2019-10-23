#pragma once

#include "infra/filesystem.h"

#include <fstream>
#include <gsl/span>
#include <optional>

namespace inf {

class CsvWriter
{
public:
    // returns the list separator as configured in the system settings or ';' otherwise
    static char system_list_separator();

    enum class NumberFormat
    {
        Scientific,
        Fixed,
    };

    struct Settings
    {
        char separator            = system_list_separator();
        NumberFormat numberFormat = NumberFormat::Fixed;
        std::optional<int32_t> precision;
        std::locale locale = std::locale::classic();
        bool utf8Bom       = false; // write utf8 bom at the start of the file
    };

    CsvWriter(const fs::path& outputPath);
    CsvWriter(const fs::path& outputPath, Settings settings);

    // Write a complete line of strings in one go
    void write_header(gsl::span<const std::string> names);
    void write_header(gsl::span<std::string_view> names);
    void write_line(gsl::span<const std::string> names);
    void write_line(gsl::span<std::string_view> names);

    // Write column by column, finish a line by calling new_line
    void write_empty_column();
    void write_column_value(std::string_view value);
    void write_column_value(int32_t value);
    void write_column_value(int64_t value);
    void write_column_value(float value);
    void write_column_value(double value);
    void new_line();

private:
    template <typename T>
    void write_value(T value);

    template <typename T>
    void write_quoted_value(T value);

    std::ofstream _stream;
    Settings _settings;
    int _fieldOffset   = 0;
    bool _quoteNumbers = false;
};

}
