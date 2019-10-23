#include "infra/csvreader.h"

namespace inf {
CsvReader::CsvReader(const fs::path& filename)
: _charset(detect_character_set(filename))
, _dataset(gdal::VectorDataSet::create(filename, gdal::VectorType::Csv))
, _layer(_dataset.layer(0))
{
}

int32_t CsvReader::column_count() const
{
    return _layer.layer_definition().field_count();
}

std::string_view CsvReader::column_name(int32_t index) const
{
    return _layer.layer_definition().field_definition(index).name();
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
    return _feature->opt_field_as<int32_t>(index);
}

std::optional<int64_t> CsvRow::get_int64(int32_t index) const noexcept
{
    return _feature->opt_field_as<int64_t>(index);
}

std::optional<double> CsvRow::get_double(int32_t index) const noexcept
{
    auto val = _feature->field_as<std::string>(index);
    if (val.empty()) {
        return {};
    }
    return CPLAtofM(val.c_str());
}

bool CsvRow::operator==(const CsvRow& other) const
{
    return _feature == other._feature;
}

CsvRowIterator::CsvRowIterator(gdal::Layer layer, inf::CharacterSet charSet)
: _iterator(std::move(layer))
, _charset(charSet)
, _currentRow(*_iterator, _charset)
{
}

const CsvRow& CsvRowIterator::operator*()
{
    return _currentRow;
}

const CsvRow* CsvRowIterator::operator->()
{
    return &_currentRow;
}

CsvRowIterator& CsvRowIterator::operator++()
{
    ++_iterator;
    _currentRow = CsvRow(*_iterator, _charset);
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
