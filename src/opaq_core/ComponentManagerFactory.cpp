#include "ComponentManagerFactory.h"

#include "ComponentManagerStatic.h"
#include "ComponentManagerDynamic.h"

namespace OPAQ
{
namespace Factory
{

std::unique_ptr<ComponentManager> createComponentManager(IEngine& engine)
{
#ifdef STATIC_PLUGINS
    return std::make_unique<ComponentManagerStatic>(engine);
#else
    return std::make_unique<ComponentManagerDynamic>(engine);
#endif
}

}
}
