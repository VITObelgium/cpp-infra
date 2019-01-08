#include "infra/gdal.h"
#include "gdx/log.h"

#include <gmock/gmock.h>

using namespace testing;

int main(int argc, char** argv)
{
    FLAGS_gmock_verbose = "error";

    gdx::Log::add_console_sink(gdx::Log::Colored::On);
    gdx::LogRegistration logReg("algotest");
    inf::gdal::Registration reg;

    InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
