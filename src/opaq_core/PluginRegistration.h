#pragma once

#include "OpaqDllExports.h"
#include "ComponentManager.h"

#include <functional>
#include <unordered_map>

namespace OPAQ
{

class PluginRegistry
{
public:
    static PluginRegistry& instance()
    {
        static PluginRegistry reg;
        return reg;
    }

    void registerPlugin(const std::string& name, FactoryCallback cb);

    std::unordered_map<std::string, FactoryCallback> _registeredFactories;
};

template <typename T>
class PluginRegistration
{
public:
    PluginRegistration(const std::string& name)
    {
        PluginRegistry::instance().registerPlugin(name, [] (LogConfiguration*) {
            return new T;
        });
    }
};

// Throws FailedToLoadPluginException PluginAlreadyLoadedException
FactoryCallback loadStaticPlugin(const std::string& pluginName, const std::string&);

// Throws FailedToLoadPluginException PluginAlreadyLoadedException
FactoryCallback loadDynamicPlugin(const std::string& pluginName, const std::string&);


}

/**
 * \brief Macro to register a class as being an OPAQ component.
 * When writing new components, a user should add this macro statement to
 * the implementation source file. Dont forget to include a correct namespace
 * if you want to have the plugin to have it's own namespace.
 *
 *   Example use: OPAQ_REGISTER_PLUGIN(OPAQ::ExampleComponent);
 */
#define OPAQ_REGISTER_DYNAMIC_PLUGIN(TYPE)                                               \
    OPAQ_DLL_API OPAQ::Component* factory(LogConfiguration* logConfig)                   \
    {                                                                                    \
        Log::initLogger(*logConfig);                                                     \
        return new TYPE();                                                               \
    }


#define OPAQ_REGISTER_STATIC_PLUGIN(TYPE)                                               \
    static auto s_pluginReg = PluginRegistration<TYPE>(TYPE::name());

