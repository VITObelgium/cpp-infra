#include <gmock/gmock.h>

#include "Logger.h"

using namespace testing;

int main(int argc, char **argv)
{
    FLAGS_gmock_verbose = "error";

    Log::initConsoleLogger();

    InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}