#pragma once

#include "Model.h"
#include "Logger.h"

namespace opaq
{

class InverseDistanceWeighting : public Model
{

public:
    InverseDistanceWeighting();

    static std::string name();

    void configure(TiXmlElement* configuration, const std::string& componentName, IEngine& engine) override;
    void run() override;

private:
    double _powerParam;
    std::string _gisType;
};

}
