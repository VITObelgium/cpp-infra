#include "infra/csvwriter.h"
#include "infra/exception.h"
#include "infra/string.h"

#include <iomanip>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

static constexpr std::array<std::uint8_t, 3> utf8Bom{{0xEF, 0xBB, 0xBF}};

namespace inf {

static char locale_requires_number_quoting(const std::locale& loc, char separator)
{
    if (std::has_facet<std::numpunct<char>>(loc)) {
        return std::use_facet<std::numpunct<char>>(loc).decimal_point() == separator ||
               std::use_facet<std::numpunct<char>>(loc).thousands_sep() == separator;
    } else {
        return false;
    }
}

static char decimal_point(const std::locale& loc)
{
    if (std::has_facet<std::numpunct<char>>(loc)) {
        char numPunct = std::use_facet<std::numpunct<char>>(loc).decimal_point();
        return numPunct;
    } else {
        return '.';
    }
}

char CsvWriter::system_list_separator()
{
#ifdef _WIN32
    char szBuf[8];
    if (::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLIST, szBuf, sizeof(szBuf)) > 0) {
        return szBuf[0];
    }
#endif

    return ',';
}

CsvWriter::CsvWriter(const fs::path& outputPath)
: CsvWriter(outputPath, Settings())
{
}

CsvWriter::CsvWriter(const fs::path& outputPath, Settings settings)
: _stream(outputPath)
, _settings(settings)
{
    if (!_stream.is_open()) {
        throw RuntimeError("Failed to open file for csv writing: {}", outputPath);
    }

    if (_settings.utf8Bom) {
        _stream.write(reinterpret_cast<const char*>(utf8Bom.data()), utf8Bom.size());
        if (_stream.bad()) {
            throw RuntimeError("Failed to write Utf8 BOM");
        }
    }

    _stream.imbue(_settings.locale);
    if (_settings.numberFormat == NumberFormat::Scientific) {
        _stream << std::scientific;
    } else {
        _stream << std::fixed;
    }

    if (_settings.precision.has_value()) {
        _stream << std::setprecision(*_settings.precision);
    }

    _quoteNumbers = locale_requires_number_quoting(_settings.locale, _settings.separator);
}

void CsvWriter::write_header(gsl::span<const std::string> names)
{
    write_line(names);
}

void CsvWriter::write_header(gsl::span<std::string_view> names)
{
    write_line(names);
}

void CsvWriter::write_line(gsl::span<const std::string> names)
{
    for (auto name : names) {
        write_column_value(name);
    }

    new_line();
}

void CsvWriter::write_line(gsl::span<std::string_view> names)
{
    for (auto name : names) {
        write_column_value(name);
    }

    new_line();
}

void CsvWriter::write_empty_column()
{
    if (_fieldOffset != 0) {
        _stream << _settings.separator;
    }

    ++_fieldOffset;
}

void CsvWriter::write_column_value(std::string_view value)
{
    bool quotesNeeded = value.find(_settings.separator) != std::string_view::npos;
    if (quotesNeeded) {
        write_quoted_value(value);
    } else {
        write_value(value);
    }
}

void CsvWriter::write_column_value(int32_t value)
{
    write_value(value);
}

void CsvWriter::write_column_value(int64_t value)
{
    write_value(value);
}

void CsvWriter::write_column_value(float value)
{
    write_value(value);
}

void CsvWriter::write_column_value(double value)
{
    write_value(value);
}

template <typename T>
void CsvWriter::write_value(T value)
{
    if constexpr (std::is_scalar_v<T>) {
        if (_quoteNumbers) {
            return write_quoted_value(value);
        }
    }

    if (_fieldOffset != 0) {
        _stream << _settings.separator;
    }

    _stream << value;
    ++_fieldOffset;
}

template <typename T>
void CsvWriter::write_quoted_value(T value)
{
    if (_fieldOffset != 0) {
        _stream << _settings.separator;
    }

    _stream << '"' << value << '"';
    ++_fieldOffset;
}

void CsvWriter::new_line()
{
    _stream << "\n";
    _fieldOffset = 0;
}
}
