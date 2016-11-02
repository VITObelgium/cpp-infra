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
namespace Config
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
    
    void setValues(Config::Component* values)
    {
        _values = values;
    }

    Config::Component& getMeteo() const
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
    Config::Component* _values;
    Config::Component* _meteo;
};

}
}
