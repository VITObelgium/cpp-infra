#include <gmock/gmock.h>

#include "Logger.h"
#include "config.h"
#include "../plugins/ForcePluginLink.h"

using namespace testing;

int main(int argc, char **argv)
{
    FLAGS_gmock_verbose = "error";

    Log::initConsoleLogger();

    // Create a logger to keep the global log registry alive
    // so it doesn't get destroyed in a shared library
    Logger logger("main");

#ifdef STATIC_PLUGINS
    for (auto& plugin : OPAQ::getPluginNames())
    {
        logger->debug("Plugin: {}", plugin);
    }
#endif

    InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
