#pragma once

#include "Logger.h"
#include "data/GridProvider.h"

#include <algorithm>

namespace OPAQ
{

class XmlGridProvider : public GridProvider
{
public:
    XmlGridProvider();

    static std::string name();

    // throws BadConfigurationException
    void configure(TiXmlElement* configuration, const std::string& componentName, IEngine& engine) override;

    virtual Grid& getGrid() override
    {
        return _grid;
    }

private:
    Grid _grid;
    Logger _logger;
};

}
