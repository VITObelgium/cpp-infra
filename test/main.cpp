#ifdef HAVE_INFRA_LOG
#include "infra/log.h"
#endif

#ifdef HAVE_GDAL
#include "infra/gdal.h"
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <cstdlib>
#include <gtest/gtest.h>

using namespace testing;

int main(int argc, char** argv)
{
#ifdef WIN32
    // make sure we can print utf8 characters in the windows console
    SetConsoleOutputCP(CP_UTF8);
#endif

#ifdef HAVE_INFRA_LOG
    inf::Log::add_console_sink(inf::Log::Colored::On);
    inf::LogRegistration logReg("InfraTest");
#endif

#ifdef HAVE_GDAL
    fs::path proj4DataPath = fs::u8path(argv[0]).parent_path() / "data";
    inf::gdal::Registration reg(proj4DataPath);
#endif

    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
