#include <gmock/gmock.h>

#include "config.h"
#include "infra/log.h"

using namespace infra;
using namespace testing;

int main(int argc, char** argv)
{
    FLAGS_gmock_verbose = "error";

    Log::addConsoleSink(Log::Colored::On);
    infra::LogRegistration logging("opaq");

    InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
