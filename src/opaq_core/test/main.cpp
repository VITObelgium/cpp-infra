#include <gmock/gmock.h>

#include "Logger.h"
#include "../plugins/ForcePluginLink.h"

using namespace testing;

int main(int argc, char **argv)
{
    FLAGS_gmock_verbose = "error";

    Log::initConsoleLogger();
    Logger log("opaqcoretest");

    for (auto& plugin : opaq::getPluginNames())
    {
        log->debug("Plugin: {}", plugin);
    }

    InitGoogleMock(&argc, argv);
    auto rc = RUN_ALL_TESTS();

    Log::destroyLogger();

    return rc;
}