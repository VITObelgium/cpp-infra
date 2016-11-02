#pragma once

#include "Logger.h"
#include "data/IGridProvider.h"

#include <algorithm>

namespace opaq
{

class XmlGridProvider : public IGridProvider
{
public:
    XmlGridProvider();

    static std::string name();

    // throws BadConfigurationException
    void configure(TiXmlElement* configuration, const std::string& componentName, IEngine& engine) override;

    const Grid& getGrid(GridType) override;

private:
    Grid _grid;
    Logger _logger;
};
}
