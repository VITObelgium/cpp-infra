#include "infra/csvreader.h"

namespace inf {
CsvReader::CsvReader(const fs::path& filename)
: _charset(detect_character_set(filename))
, _dataset(gdal::VectorDataSet::create(filename, gdal::VectorType::Csv))
, _layer(_dataset.layer(0))
{
}

int32_t CsvReader::column_count()
{
    return _layer.layer_definition().field_count();
}

CsvRowIterator CsvReader::begin() const
{
    return CsvRowIterator(_layer, _charset);
}

CsvRowIterator CsvReader::end() const
{
    return CsvRowIterator();
}

CsvRow::CsvRow(const gdal::Feature& feat, inf::CharacterSet charSet)
: _feature(&feat)
, _charSet(charSet)
{
}

bool CsvRow::column_is_empty(int32_t index) const noexcept
{
    return _feature->field_as<std::string_view>(index).empty();
}

std::string CsvRow::get_string(int32_t index) const noexcept
{
    if (_charSet == CharacterSet::Utf8) {
        return std::string(_feature->field_as<std::string_view>(index));
    } else {
        return convert_to_utf8(_feature->field_as<std::string_view>(index));
    }
}

std::optional<int32_t> CsvRow::get_int32(int32_t index) const noexcept
{
    return _feature->field_as<int32_t>(index);
}

std::optional<int64_t> CsvRow::get_int64(int32_t index) const noexcept
{
    return _feature->field_as<int64_t>(index);
}

std::optional<double> CsvRow::get_double(int32_t index) const noexcept
{
    return CPLAtofM(_feature->field_as<std::string>(index).c_str());
}

bool CsvRow::operator==(const CsvRow& other) const
{
    return _feature == other._feature;
}

CsvRowIterator::CsvRowIterator(gdal::Layer layer, inf::CharacterSet charSet)
: _iterator(std::move(layer))
, _charset(charSet)
{
}

CsvRow CsvRowIterator::operator*()
{
    return CsvRow(*_iterator, _charset);
}

CsvRowIterator& CsvRowIterator::operator++()
{
    ++_iterator;
    return *this;
}

CsvRowIterator& CsvRowIterator::operator=(CsvRowIterator&& other)
{
    if (this != &other) {
        _iterator = std::move(other._iterator);
    }

    return *this;
}

bool CsvRowIterator::operator==(const CsvRowIterator& other) const
{
    return _iterator == other._iterator;
}

bool CsvRowIterator::operator!=(const CsvRowIterator& other) const
{
    return !(*this == other);
}

}
