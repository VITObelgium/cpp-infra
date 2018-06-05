#pragma once

#include "PluginFactoryInterface.h"

namespace opaq {
class PluginFactory : public IPluginFactory
{
public:
    std::unique_ptr<Component> createPlugin(std::string_view name) const override;
};
}
