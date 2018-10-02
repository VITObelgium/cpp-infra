#pragma once

#include "data/IGridProvider.h"

#include <algorithm>
#include <map>

namespace opaq {

class TextGridProvider : public IGridProvider
{
public:
    static std::string name();

    // throws BadConfigurationException
    void configure(const inf::XmlNode& configuration, const std::string& componentName, IEngine& engine) override;

    const Grid& getGrid(const std::string& pollutant, GridType type) override;

private:
    void readFile(const std::string& pollutant, GridType type);

    std::string _pattern;
    std::map<std::string, std::map<GridType, Grid>> _grid;
};
}
