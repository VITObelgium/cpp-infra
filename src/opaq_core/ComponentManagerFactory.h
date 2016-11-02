#pragma once

#include "ComponentManager.h"

#include <memory>

namespace opaq
{

class IEngine;

namespace Factory
{

ComponentManager createComponentManager(IEngine&);

}

}
