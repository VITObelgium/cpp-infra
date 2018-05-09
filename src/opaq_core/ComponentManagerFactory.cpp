#include "ComponentManagerFactory.h"

#include "PluginRegistration.h"
#include "config.h"

namespace opaq {
namespace factory {

ComponentManager createComponentManager(IEngine& engine)
{
    return ComponentManager(engine, loadStaticPlugin);
}

}
}
