#pragma once

#include "infra/cast.h"
#include "infra/charset.h"
#include "infra/gdal.h"
#include "infra/typetraits.h"
#include <string>

namespace inf {

class CsvRow
{
public:
    CsvRow() = default;
    CsvRow(const gdal::Feature& feat, inf::CharacterSet charSet);

    bool column_is_empty(int32_t index) const noexcept;

    template <typename T>
    std::optional<T> get_column_as(int32_t index) const noexcept
    {
        if constexpr (std::is_same_v<T, std::string>) {
            return get_string(index);
        } else if constexpr (std::is_same_v<T, int32_t>) {
            return get_int32(index);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return get_int64(index);
        } else if constexpr (std::is_same_v<T, double>) {
            return get_double(index);
        } else if constexpr (std::is_same_v<T, float>) {
            return optional_cast<float>(get_double(index));
        } else {
            static_assert(dependent_false<T>::value, "Unsupported csv type provided");
        }
    }

    std::string get_string(int32_t index) const noexcept;
    std::optional<int32_t> get_int32(int32_t index) const noexcept;
    std::optional<int64_t> get_int64(int32_t index) const noexcept;
    std::optional<double> get_double(int32_t index) const noexcept;

    bool operator==(const CsvRow& other) const;

private:
    const gdal::Feature* _feature = nullptr;
    inf::CharacterSet _charSet    = CharacterSet::Unknown;
};

// Iteration is not thread safe!
// Do not iterate simultaneously from different threads.
class CsvRowIterator
{
public:
    CsvRowIterator() = default;
    CsvRowIterator(gdal::Layer layer, inf::CharacterSet charSet);

    CsvRowIterator& operator++();
    CsvRowIterator& operator=(CsvRowIterator&& other);
    bool operator==(const CsvRowIterator& other) const;
    bool operator!=(const CsvRowIterator& other) const;
    const CsvRow& operator*();
    const CsvRow* operator->();

private:
    gdal::LayerIterator<gdal::Layer> _iterator;
    inf::CharacterSet _charset = CharacterSet::Unknown;
    CsvRow _currentRow;
};

class CsvReader
{
public:
    CsvReader(const fs::path& filename);
    CsvReader(const CsvReader&)            = delete;
    CsvReader(CsvReader&&)                 = default;
    CsvReader& operator=(const CsvReader&) = delete;
    CsvReader& operator=(CsvReader&&)      = default;

    int32_t column_count() const;
    std::string_view column_name(int32_t index) const;
    std::optional<int32_t> column_index(const std::string& name) const noexcept;
    int32_t required_column_index(const std::string& name) const;

    CsvRowIterator begin() const;
    CsvRowIterator end() const;

private:
    inf::CharacterSet _charset;
    gdal::VectorDataSet _dataset;
    gdal::Layer _layer;
};

// support for range based for loops
inline CsvRowIterator begin(const CsvReader& reader)
{
    return reader.begin();
}

inline CsvRowIterator end(const CsvReader& reader)
{
    return reader.end();
}

}
