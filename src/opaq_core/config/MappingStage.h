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

namespace OPAQ
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
    
    void setValues(OPAQ::Config::Component* values)
    {
        _values = values;
    }

    OPAQ::Config::Component& getMeteo() const
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
    OPAQ::Config::Component* _values;
    OPAQ::Config::Component* _meteo;
};

}
}
