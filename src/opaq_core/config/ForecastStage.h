#pragma once

#include "../DateTime.h"
#include "Component.h"

#include <vector>

namespace opaq
{

namespace config
{

class ForecastStage
{
public:
    ForecastStage(chrono::days fcHor,
                  config::Component values,
                  config::Component meteo,
                  config::Component buffer,
                  config::Component outputWriter,
                  std::vector<Component> models);

    config::Component getValues() const;
    config::Component getMeteo() const;
    config::Component getBuffer() const;
    config::Component getOutputWriter() const;
    const std::vector<Component>& getModels() const noexcept;

    chrono::days getHorizon() const noexcept;

private:
    chrono::days _fcHor; //!< requested max forecast horizon

    Component _values;
    Component _meteo;

    Component _buffer;
    Component _outputWriter;

    std::vector<Component> _models;
};

}
}
