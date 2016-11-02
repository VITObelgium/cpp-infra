#pragma once

#include "ComponentManager.h"

#include <memory>

namespace opaq
{

class IEngine;

namespace factory
{

ComponentManager createComponentManager(IEngine&);

}

}
