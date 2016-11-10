#pragma once

#include "../DateTime.h"
#include "Component.h"

#include <vector>
#include <boost/optional.hpp>

namespace opaq
{

namespace config
{

class ForecastStage
{
public:
    ForecastStage(chrono::days fcHor,
                  config::Component values,
                  config::Component buffer,
                  config::Component outputWriter,
                  boost::optional<config::Component> meteo,
                  std::vector<Component> models);

    config::Component getValues() const;
    config::Component getBuffer() const;
    config::Component getOutputWriter() const;
    boost::optional<config::Component> getMeteo() const;
    const std::vector<Component>& getModels() const noexcept;

    chrono::days getHorizon() const noexcept;

private:
    chrono::days _fcHor; //!< requested max forecast horizon

    Component _values;
    Component _buffer;
    Component _outputWriter;

    boost::optional<config::Component> _meteo;

    std::vector<Component> _models;
};

}
}
