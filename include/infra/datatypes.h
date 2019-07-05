#pragma once

#include "infra/color.h"

#include <chrono>
#include <optional>
#include <string>

namespace inf {

struct ValueMapping
{
    ValueMapping() = default;
    ValueMapping(int64_t id_, std::string_view key_, double scaleValue_, std::string_view remark_, std::string_view unit_)
    : id(id_)
    , key(key_)
    , scaleValue(scaleValue_)
    , remark(remark_)
    , unit(unit_)
    {
    }

    ValueMapping(int64_t id_, double lowerLimit_, std::optional<double> upperLimit_, double scaleValue_, std::string_view remark_, std::string_view unit_)
    : id(id_)
    , lowerLimit(lowerLimit_)
    , upperLimit(upperLimit_)
    , scaleValue(scaleValue_)
    , remark(remark_)
    , unit(unit_)
    {
    }

    std::optional<int64_t> id;        // if empty, this ValueMapping isn't present in the database yet
    int64_t idRc;
    std::string key;                  // if not empty, string based mapping
    double lowerLimit = 0.0;          // if upperlimit empty, discrete mapping
    std::optional<double> upperLimit; // if not empty, range based mapping
    double scaleValue;
    std::string remark;
    std::string unit;
};
struct ColorMapping
{
    double lowerLimit;
    double upperLimit;
    inf::Color color;
};

struct ConfigParameter
{
    std::string name;
    std::string type;
    std::string value;
};
}