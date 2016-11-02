#include "ComponentManagerFactory.h"

#include "config.h"
#include "PluginRegistration.h"

namespace opaq
{
namespace Factory
{

ComponentManager createComponentManager(IEngine& engine)
{
#ifdef STATIC_PLUGINS
    return ComponentManager(engine, loadStaticPlugin);
#else
    return ComponentManager(engine, loadDynamicPlugin);
#endif
}

}
}
