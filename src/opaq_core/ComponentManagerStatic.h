#pragma once

#include "ComponentManager.h"

namespace OPAQ
{

class ComponentManagerStatic : public ComponentManager
{
public:
    ComponentManagerStatic(IEngine& engine);

    // throws FailedToLoadPluginException, PluginAlreadyLoadedException
    void loadPlugin(const std::string& pluginName, const std::string& filename) override;

    static void registerPlugin(const std::string& name, FactoryCallback cb);

private:
    static FactoryMapType _registeredFactories;
};

}
