#include "infra/gdal.h"
#include "infra/log.h"
#include "testconfig.h"

#include <cstdlib>
#include <gmock/gmock.h>

using namespace testing;

#ifdef WIN32
#define putenv _putenv
#endif

int main(int argc, char** argv)
{
    FLAGS_gmock_verbose = "error";
    if (getenv("GDAL_DATA") == nullptr) {
        static char env[] = "GDAL_DATA=" GDAL_DATA;
        putenv(env);
    }

    inf::Log::add_console_sink(inf::Log::Colored::On);
    inf::LogRegistration logReg("GdxUtilTest");
    inf::gdal::Registration reg;

    InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
