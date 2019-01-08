#pragma once

#include "gdx/exception.h"
#include "gdx/log.h"

#include <algorithm>
#include <fstream>
#include <gsl/span>
#include <limits>
#include <map>
#include <set>
#include <sstream>

namespace gdx {

enum class Operation
{
    Sum,
    Count,
    CountNonZero,
    Min,
    Max,
    Average,
    AverageInCat1
};

template <template <typename> typename RasterType, typename T>
void tableRow(const RasterType<T>& ras, const RasterType<T>& categoryRaster, Operation op, const std::string& outputPath, const std::string& label, bool append)
{
    const auto rows = ras.rows();
    const auto cols = ras.cols();
    std::map<int, std::pair<double, int>> cat;

    if (op == Operation::AverageInCat1) {
        cat[1] = std::pair<double, int>(0, 0);
    }

    int countNodata = 0;
    std::set<int> catWithNodata;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int currentCategory = static_cast<int>(categoryRaster(r, c));
            auto currentValue   = ras(r, c);

            if (!categoryRaster.is_nodata(r, c) && !ras.is_nodata(r, c)) {
                auto iter = cat.find(currentCategory);

                if (iter == cat.end()) {
                    if (op != Operation::AverageInCat1) {
                        cat.emplace(currentCategory, std::pair<double, int>(static_cast<double>(currentValue), 1));
                    } else {
                        assert(currentCategory != 1);
                        // ignore all other categories (like category 0)
                    }
                } else {
                    if (op == Operation::Sum ||
                        op == Operation::Average ||
                        ((op == Operation::AverageInCat1) && (currentCategory == 1))) {
                        iter->second.first += currentValue;
                        ++iter->second.second;
                    } else if (op == Operation::Min) {
                        if (iter->second.first > currentValue) {
                            iter->second.first = currentValue;
                        }
                    } else if (op == Operation::Max) {
                        if (iter->second.first < currentValue) {
                            iter->second.first = currentValue;
                        }
                    } else {
                        assert(op == Operation::AverageInCat1 && currentCategory != 1);
                    }
                }
            } else if (ras.is_nodata(r, c) && !categoryRaster.is_nodata(r, c)) {
                ++countNodata;
                catWithNodata.insert(currentCategory);
                auto iter = cat.find(currentCategory);
                if (iter == cat.end()) {
                    if (op != Operation::AverageInCat1) {
                        cat.emplace(currentCategory, std::make_pair<double, int>(0.0, 0));
                    }
                }
            }
        }
    }

    if (countNodata) {
        Log::warn("tablerow(xls,op,A,B): map A contains {} NODATA values where B is a normal nonzero value\nThese values of A are ignored in the output table", countNodata);
    }

    if (op != Operation::AverageInCat1) {
        for (auto& elem : catWithNodata) {
            if (cat.find(elem) == cat.end()) {
                cat.emplace(elem, std::pair<double, int>(std::numeric_limits<double>::quiet_NaN(), 1));
            }
        }
    }

    std::ofstream fs(outputPath.c_str(), append ? std::ios_base::out | std::ios_base::app : std::ios_base::out);
    if (!fs.is_open()) {
        throw RuntimeError("Cannot create file {}", outputPath);
    }

    if (!append) {
        fs << "cat";
        for (auto iter = cat.begin(); iter != cat.end(); ++iter) {
            fs << "\t" << iter->first;
        }
        if (cat.empty()) {
            fs << "\t"; // print an empty column, that prevents problems with automatic processing those reults
        }
        fs << "\n";
    }

    fs << label;
    for (auto& [catKey, value] : cat) {
        (void) catKey;
        if (op == Operation::AverageInCat1) {
            if (value.second > 0 && !std::isnan(value.first)) {
                fs << "\t" << value.first / value.second;
            } else {
                fs << "\t";
            }
        } else {
            if (value.second > 0 && !std::isnan(value.first)) {
                fs << "\t" << (op != Operation::Average ? value.first : value.first / value.second);
            } else {
                fs << "\t=NA()";
            }
        }
    }
    if (cat.empty()) {
        fs << "\t"; // print an empty column, that prevents problems with automatic processing those reults
    }
    fs << "\n";
}

}
