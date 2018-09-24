#ifdef HAVE_INFRA_LOG
#include "infra/log.h"
#endif

#ifdef HAVE_GDAL
#include "infra/gdal.h"
#endif

#include <cstdlib>
#include <gtest/gtest.h>

using namespace testing;

int main(int argc, char** argv)
{
#ifdef HAVE_INFRA_LOG
    inf::Log::add_console_sink(inf::Log::Colored::On);
    inf::LogRegistration logReg("InfraTest");
#endif

#ifdef HAVE_GDAL
    inf::gdal::Registration reg;
#endif

    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
