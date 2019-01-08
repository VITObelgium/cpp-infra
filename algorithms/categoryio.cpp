#include "gdx/algo/categoryio.h"
#include "gdx/exception.h"
#include "gdx/log.h"
#include "infra/gdal.h"

#include <fmt/format.h>
#include <functional>
#include <ogrsf_frmts.h>

namespace gdx::detail {

namespace gdal = inf::gdal;

std::vector<std::vector<double>> readTabDataRowBased(const std::string& fileName)
{
    // Files not ending in csv need to be prepended with CSV: to be treated as such
    return readCsvDataRowBased(fmt::format("CSV:{}", fileName.data()));
}

std::vector<std::vector<double>> readTabDataColumnBased(const std::string& fileName)
{
    // Files not ending in csv need to be prepended with CSV: to be treated as such
    return readCsvDataColumnBased(fmt::format("CSV:{}", fileName.data()));
}

std::vector<std::vector<double>> readCsvDataRowBased(const std::string& fileName)
{
    std::vector<std::vector<double>> result;

    auto dataSet = gdal::VectorDataSet::create(fileName, gdal::VectorType::Csv);
    if (dataSet.layer_count() != 1) {
        throw RuntimeError("Only tab files with a single band are currently supported");
    }

    for (auto& feature : dataSet.layer(0)) {
        std::vector<double> row;
        row.reserve(feature.field_count());

        for (auto& field : feature) {
            if (std::holds_alternative<int32_t>(field)) {
                row.push_back(std::get<int32_t>(field));
            } else if (std::holds_alternative<int64_t>(field)) {
                row.push_back(static_cast<double>(std::get<int64_t>(field)));
            } else if (std::holds_alternative<double>(field)) {
                row.push_back(std::get<double>(field));
            } else if (std::holds_alternative<std::string_view>(field)) {
                row.push_back(std::atof(std::get<std::string_view>(field).data()));
            } else {
                Log::warn("Unrecognized csv entry");
            }
        }

        result.emplace_back(std::move(row));
    }

    return result;
}

std::vector<std::vector<double>> readCsvDataColumnBased(const std::string& fileName)
{
    std::vector<std::vector<double>> result;

    auto dataSet = gdal::VectorDataSet::create(fileName, gdal::VectorType::Csv);
    if (dataSet.layer_count() != 1) {
        throw RuntimeError("Only tab files with a single band are currently supported");
    }

    for (auto& feature : dataSet.layer(0)) {
        if (feature.field_count() > static_cast<int32_t>(result.size())) {
            result.resize(feature.field_count());
        }

        int index = 0;
        for (auto& field : feature) {
            if (std::holds_alternative<int32_t>(field)) {
                result[index].push_back(std::get<int32_t>(field));
            } else if (std::holds_alternative<int64_t>(field)) {
                result[index].push_back(static_cast<double>(std::get<int64_t>(field)));
            } else if (std::holds_alternative<double>(field)) {
                result[index].push_back(std::get<double>(field));
            } else if (std::holds_alternative<std::string_view>(field)) {
                auto doubleStr = std::get<std::string_view>(field);
                char* end;
                double value = std::strtod(doubleStr.data(), &end);
                if (errno == ERANGE || end == doubleStr.data()) {
                    throw RuntimeError("Invalid value in mapping file: '{}'", doubleStr);
                }
                result[index].push_back(value);
            } else {
                Log::warn("Unrecognized csv entry");
            }

            ++index;
        }
    }

    return result;
}
}
