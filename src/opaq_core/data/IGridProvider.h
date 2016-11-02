#pragma once

#include "../Component.h"
#include "../Grid.h"
#include "../Exceptions.h"

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

class IGridProvider : public Component
{
public:
    virtual const Grid& getGrid(GridType type) = 0;
};

}
