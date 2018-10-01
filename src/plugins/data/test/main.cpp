#include <gmock/gmock.h>

#include "infra/log.h"
#include "opaqconfig.h"

using namespace inf;
using namespace testing;

int main(int argc, char** argv)
{
    FLAGS_gmock_verbose = "error";

    Log::add_console_sink(Log::Colored::On);
    inf::LogRegistration logging("opaq");

    InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
