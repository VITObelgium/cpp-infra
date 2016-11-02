#pragma once

#include "Logger.h"
#include "data/IGridProvider.h"

#include <algorithm>

namespace opaq
{

class TextGridProvider : public IGridProvider
{
public:
    TextGridProvider();

    static std::string name();

    // throws BadConfigurationException
    void configure(TiXmlElement* configuration, const std::string& componentName, IEngine& engine) override;

    const Grid& getGrid(GridType type) override;

private:
    void readFile(GridType type);

    Logger _logger;
    std::string _pattern;
    std::map<GridType, Grid> _grid;
};
}
