#pragma once

#include <string>
#include <vector>

namespace gdx::detail {

// Row based (inner vector contains the rows)
std::vector<std::vector<double>> readTabDataRowBased(const std::string& fileName);
std::vector<std::vector<double>> readCsvDataRowBased(const std::string& fileName);

// Column based (inner vector contains the columns)
std::vector<std::vector<double>> readTabDataColumnBased(const std::string& fileName);
std::vector<std::vector<double>> readCsvDataColumnBased(const std::string& fileName);

} // namespace gdx::io
