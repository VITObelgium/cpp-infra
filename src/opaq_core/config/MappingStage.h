/*
 * Plugin.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#pragma once

#include <string>
#include <tinyxml.h>
#include <vector>

#include "../Exceptions.h"
#include "../tools/ExceptionTools.h"
#include "Component.h"

namespace opaq
{
namespace config
{

class MappingStage
{
public:
    MappingStage();

    Component& getValues() const
    {
        throwOnNullPtr(_values);
        return *_values;
    }
    
    void setValues(config::Component* values)
    {
        _values = values;
    }

    config::Component& getMeteo() const
    {
        throwOnNullPtr(_meteo);
        return *_meteo;
    }
    
    void setMeteo(Component* meteo)
    {
        _meteo = meteo;
    }

private:
    // input data provider components
    config::Component* _values;
    config::Component* _meteo;
};

}
}
