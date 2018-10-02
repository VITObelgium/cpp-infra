#pragma once

#include "ComponentManager.h"

#include <string_view>

namespace inf {
class XmlNode;
}

namespace opaq {
class IPluginFactory
{
public:
    virtual ~IPluginFactory() = default;

    virtual std::unique_ptr<Component> createPlugin(std::string_view name) const = 0;
};
}
