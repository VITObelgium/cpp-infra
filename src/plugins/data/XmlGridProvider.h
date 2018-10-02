#pragma once

#include "data/IGridProvider.h"

#include <algorithm>

namespace opaq {

class XmlGridProvider : public IGridProvider
{
public:
    static std::string name();

    // throws BadConfigurationException
    void configure(const inf::XmlNode& configuration, const std::string& componentName, IEngine& engine) override;

    const Grid& getGrid(const std::string& pollutant, GridType) override;

private:
    Grid _grid;
};
}
