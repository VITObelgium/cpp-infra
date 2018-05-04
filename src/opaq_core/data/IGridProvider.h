#pragma once

#include "Component.h"
#include "Grid.h"
#include "GridType.h"
#include "Exceptions.h"

namespace opaq
{

class IGridProvider : public Component
{
public:
    virtual const Grid& getGrid(const std::string& pollutant, GridType type) = 0;
};

}
