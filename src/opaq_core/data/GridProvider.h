#pragma once

#include "../Component.h"
#include "../Grid.h"

namespace opaq
{

class GridProvider : public Component
{
public:
    virtual Grid& getGrid() = 0;
};

}
