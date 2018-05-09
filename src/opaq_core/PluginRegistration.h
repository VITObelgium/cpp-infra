#pragma once

#include "ComponentManager.h"

#include <functional>
#include <unordered_map>

namespace opaq {

class PluginRegistry
{
public:
    static PluginRegistry& instance()
    {
        static PluginRegistry reg;
        return reg;
    }

    void registerPlugin(const std::string& name, FactoryCallback cb);
    FactoryCallback getPluginFactory(const std::string& name);

private:
    std::unordered_map<std::string, FactoryCallback> _registeredFactories;
};

template <typename T>
class PluginRegistration
{
public:
    PluginRegistration(const std::string& name)
    {
        PluginRegistry::instance().registerPlugin(name, [](LogConfiguration*) {
            return new T;
        });
    }
};

// Throws FailedToLoadPluginException PluginAlreadyLoadedException
FactoryCallback loadStaticPlugin(const std::string& pluginName);

}

#define OPAQ_REGISTER_STATIC_PLUGIN(TYPE) \
    static auto s_pluginReg = PluginRegistration<TYPE>(TYPE::name());
