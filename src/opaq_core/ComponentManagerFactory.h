#pragma once

#include <memory>

namespace OPAQ
{

class IEngine;
class ComponentManager;

namespace Factory
{

std::unique_ptr<ComponentManager> createComponentManager(IEngine&);

}

}
