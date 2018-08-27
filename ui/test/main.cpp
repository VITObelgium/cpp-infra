#include "infra/gdal.h"
#include "infra/log.h"

#include <cstdlib>
#include <gtest/gtest.h>

using namespace testing;

int main(int argc, char** argv)
{
    inf::Log::addConsoleSink(inf::Log::Colored::On);
    inf::LogRegistration logReg("UiInfraTest");

    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
