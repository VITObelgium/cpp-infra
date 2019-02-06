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

CsvLineIterator CsvReader::begin() const
{
    return CsvLineIterator(_layer, _charset);
}

CsvLineIterator CsvReader::end() const
{
    return CsvLineIterator();
}

CsvLine::CsvLine(const gdal::Feature& feat, inf::CharacterSet charSet)
: _feature(&feat)
, _charSet(charSet)
{
}

std::string CsvLine::get_string(int32_t index) const noexcept
{
    if (_charSet == CharacterSet::Utf8) {
        return std::string(_feature->field_as<std::string_view>(index));
    } else {
        return convert_to_utf8(_feature->field_as<std::string_view>(index));
    }
}

std::optional<int32_t> CsvLine::get_int32(int32_t index) const noexcept
{
    return _feature->field_as<int32_t>(index);
}

std::optional<int64_t> CsvLine::get_int64(int32_t index) const noexcept
{
    return _feature->field_as<int64_t>(index);
}

std::optional<double> CsvLine::get_double(int32_t index) const noexcept
{
    return _feature->field_as<double>(index);
}

bool CsvLine::operator==(const CsvLine& other) const
{
    return _feature == other._feature;
}

CsvLineIterator::CsvLineIterator(gdal::Layer layer, inf::CharacterSet charSet)
: _iterator(std::move(layer))
, _charset(charSet)
{
}

CsvLine CsvLineIterator::operator*()
{
    return CsvLine(*_iterator, _charset);
}

CsvLineIterator& CsvLineIterator::operator++()
{
    ++_iterator;
    return *this;
}

CsvLineIterator& CsvLineIterator::operator=(CsvLineIterator&& other)
{
    if (this != &other) {
        _iterator = std::move(other._iterator);
    }

    return *this;
}

bool CsvLineIterator::operator==(const CsvLineIterator& other) const
{
    return _iterator == other._iterator;
}

bool CsvLineIterator::operator!=(const CsvLineIterator& other) const
{
    return !(*this == other);
}

}
