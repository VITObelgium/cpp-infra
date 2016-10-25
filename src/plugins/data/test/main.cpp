#include <gmock/gmock.h>

#include "Logger.h"

using namespace testing;

int main(int argc, char **argv)
{
    FLAGS_gmock_verbose = "error";

    Log::initConsoleLogger();
    
    // Create a logger to keep the global log registry alive
    // so it doesn't get destroyed in a shared library
    Logger logger("main");

    InitGoogleMock(&argc, argv);
    auto rc = RUN_ALL_TESTS();
    spdlog::drop_all();
    return rc;
}
