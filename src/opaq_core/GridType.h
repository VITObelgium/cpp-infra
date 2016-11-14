#pragma once

#include "Exceptions.h"

#include <string>

namespace opaq
{

enum class GridType
{
    Grid1x1,
    Grid4x4
};

inline std::string gridTypeToString(GridType type)
{
    switch (type)
    {
    case GridType::Grid1x1: return "1x1";
    case GridType::Grid4x4: return "4x4";
    default:
        throw InvalidArgumentsException("Invalid grid type provided");
    }
}

inline GridType gridTypeFromString(const std::string& gridType)
{
    if (gridType == "1x1")  return GridType::Grid1x1;
    if (gridType == "4x4")  return GridType::Grid4x4;

    throw InvalidArgumentsException("Invalid grid type provided: {}", gridType);
}

inline size_t gridTypeToCellSize(GridType type)
{
    switch (type)
    {
    case GridType::Grid1x1: return 1000;
    case GridType::Grid4x4: return 4000;
    default:
        throw InvalidArgumentsException("Invalid grid type provided");
    }
}

}
