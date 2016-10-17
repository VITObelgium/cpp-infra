#pragma once

#include "ComponentManager.h"

namespace OPAQ
{

class ComponentManagerDynamic : public ComponentManager
{
public:
    ComponentManagerDynamic(IEngine& engine);

    // throws FailedToLoadPluginException, PluginAlreadyLoadedException
    void loadPlugin(const std::string& pluginName, const std::string& filename) override;
};

}
