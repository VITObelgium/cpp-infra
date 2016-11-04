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
    ForecastStage();

    const config::Component& getValues() const;
    void setValues(const config::Component& values);

    const config::Component& getMeteo() const;
    void setMeteo(const config::Component& meteo);
    
    const config::Component& getBuffer() const;
    void setBuffer(const config::Component& buffer);
    
    const config::Component& getOutputWriter() const;
    void setOutputWriter(const Component& ow);

    void addModel(const Component& model);
    const std::vector<Component>& getModels() const noexcept;

    void setHorizon(chrono::days fcHor);
    chrono::days getHorizon() const noexcept;

private:
    // vector of models to run in the forecast
    std::vector<Component> _models;

    // input data provider components
    const Component* _values;
    const Component* _meteo;

    // forecast buffer component
    const Component* _buffer;

    // output writer component
    const Component* _outputWriter;

    chrono::days _fcHor; //!< requested max forecast horizon
};

}
}
