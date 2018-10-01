#include <gmock/gmock.h>

#include "infra/log.h"

using namespace inf;
using namespace testing;

int main(int argc, char** argv)
{
    FLAGS_gmock_verbose = "error";

    Log::add_console_sink(Log::Colored::On);
    Log::initialize("opaqcoretest");

    InitGoogleMock(&argc, argv);
    auto rc = RUN_ALL_TESTS();

    Log::uninitialize();

    return rc;
}
