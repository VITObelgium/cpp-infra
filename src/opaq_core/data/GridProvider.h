#pragma once

#include "../Component.h"
#include "../Grid.h"

namespace OPAQ
{

class GridProvider : public Component
{
public:
    virtual Grid& getGrid() = 0;
};

}
