#pragma once

#include "infra/cast.h"
#include "infra/charset.h"
#include "infra/gdal.h"
#include "infra/typetraits.h"

namespace inf {

class CsvLine
{
public:
    CsvLine() = default;
    CsvLine(const gdal::Feature& feat, inf::CharacterSet charSet);

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

    bool operator==(const CsvLine& other) const;

private:
    const gdal::Feature* _feature = nullptr;
    inf::CharacterSet _charSet    = CharacterSet::Unknown;
};

// Iteration is not thread safe!
// Do not iterate simultaneously from different threads.
class CsvLineIterator
{
public:
    CsvLineIterator() = default;
    CsvLineIterator(gdal::Layer layer, inf::CharacterSet charSet);
    CsvLineIterator(const CsvLineIterator&) = delete;
    CsvLineIterator(CsvLineIterator&&)      = default;

    CsvLineIterator& operator++();
    CsvLineIterator& operator=(CsvLineIterator&& other);
    bool operator==(const CsvLineIterator& other) const;
    bool operator!=(const CsvLineIterator& other) const;
    CsvLine operator*();

private:
    gdal::LayerIterator _iterator;
    inf::CharacterSet _charset = CharacterSet::Unknown;
};

class CsvReader
{
public:
    CsvReader(const fs::path& filename);
    CsvReader(const CsvReader&) = delete;
    CsvReader(CsvReader&&)      = default;
    CsvReader& operator=(const CsvReader&) = delete;
    CsvReader& operator=(CsvReader&&) = default;

    int32_t column_count();

    CsvLineIterator begin() const;
    CsvLineIterator end() const;

private:
    inf::CharacterSet _charset;
    gdal::VectorDataSet _dataset;
    gdal::Layer _layer;
};

// support for range based for loops
inline CsvLineIterator begin(const CsvReader& reader)
{
    return reader.begin();
}

inline CsvLineIterator end(const CsvReader& reader)
{
    return reader.end();
}

}
