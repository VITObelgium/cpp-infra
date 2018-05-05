#pragma once

#include "Logger.h"
#include "data/IGridProvider.h"

#include <algorithm>

namespace opaq {

class TextGridProvider : public IGridProvider
{
public:
    TextGridProvider();

    static std::string name();

    // throws BadConfigurationException
    void configure(const infra::ConfigNode& configuration, const std::string& componentName, IEngine& engine) override;

    const Grid& getGrid(const std::string& pollutant, GridType type) override;

private:
    void readFile(const std::string& pollutant, GridType type);

    Logger _logger;
    std::string _pattern;
    std::map<std::string, std::map<GridType, Grid>> _grid;
};
}
