#include "gdx/algo/reclass.h"
#include "gdx/algo/categoryio.h"

namespace gdx {

std::vector<std::vector<double>> readMappingFile(const std::string& mappingFile, int32_t dataColumns)
{
    auto table = detail::readTabDataRowBased(mappingFile);

    // the number of columns in the mapping table should match the number of rasters provided + 1
    if (static_cast<int32_t>(table.at(0).size()) != dataColumns + 1) {
        throw RuntimeError("The mapping table size should match the number of rasters provided");
    }

    auto size = table.front().size();
    for (auto& row : table) {
        if (row.size() != size) {
            throw RuntimeError("Not all rows in the mapping table have the same size");
        }

        // Compatibility with old behavior, replace -9999 values with nan
        std::replace(row.begin(), row.end(), -9999.0, std::numeric_limits<double>::quiet_NaN());
    }

    return table;
}

std::vector<std::vector<double>> readMappingFile(const std::string& mappingFile, int32_t dataColumns, int32_t index)
{
    auto table = detail::readTabDataColumnBased(mappingFile);

    if (index == 0) {
        throw InvalidArgument("Invalid index (0), first index is 1");
    }

    if (dataColumns + index > static_cast<int32_t>(table.size())) {
        throw InvalidArgument("Invalid index in mapping file {}, available mappings is {}", table.size());
    }

    auto size = table.front().size();
    if (!std::all_of(table.begin(), table.end(), [size](auto& vec) { return vec.size() == size; })) {
        throw RuntimeError("Not all rows in the mapping table have the same size");
    }

    const auto& mapTo = table[(dataColumns - 1) + index];

    std::vector<std::vector<double>> mapping;
    mapping.reserve(size);

    for (size_t i = 0; i < size; ++i) {
        std::vector<double> values(dataColumns + 1);
        for (int j = 0; j < dataColumns; ++j) {
            auto value = table[j][i];
            if (value == -9999.0) {
                value = std::numeric_limits<double>::quiet_NaN();
            }

            values[j] = value;
        }

        auto v2 = mapTo[i];
        if (v2 == -9999.0) {
            v2 = std::numeric_limits<double>::quiet_NaN();
        }

        values[dataColumns] = v2;
        mapping.emplace_back(std::move(values));
    }

    return mapping;
}

std::vector<std::vector<double>> readNMappingFile(const std::string& mappingFile)
{
    auto data = detail::readTabDataRowBased(mappingFile);
    for (auto& row : data) {
        if (row.size() != 3) {
            throw RuntimeError("Each row in the nreclass mapping file should contain 3 values");
        }

        // Compatibility with old behavior, replace -9999 values with nan
        std::replace(row.begin(), row.end(), -9999.0, std::numeric_limits<double>::quiet_NaN());
    }

    return data;
}
}
