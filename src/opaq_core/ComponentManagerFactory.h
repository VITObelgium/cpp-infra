#pragma once

#include "ComponentManager.h"

#include <memory>

namespace OPAQ
{

class IEngine;

namespace Factory
{

ComponentManager createComponentManager(IEngine&);

}

}
