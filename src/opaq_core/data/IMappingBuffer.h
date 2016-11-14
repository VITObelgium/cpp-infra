#pragma once

#include "Component.h"
#include "Aggregation.h"
#include "DateTime.h"
#include "GridType.h"

#include <vector>

namespace opaq
{

class Grid;
class Station;
class Pollutant;

class IMappingBuffer : public Component
{
public:
    virtual void openResultsFile(chrono::date_time begin, chrono::date_time end,
                                 const Pollutant& pol, Aggregation::Type agg,
                                 const std::vector<Station>& stations, const Grid& grid, GridType gridType) = 0;
    virtual void addResults(const std::vector<double>& results) = 0;
    virtual void closeResultsFile() = 0;
};
}
