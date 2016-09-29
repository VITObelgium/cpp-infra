#include "Component.h"

namespace OPAQ
{

std::string Component::getName() const noexcept
{
    return _name;
}

void Component::setName(const std::string& componentName)
{ 
    _name = componentName;
}

}
