#define DOCTEST_CONFIG_IMPLEMENT

#include "infra/test/reporter.h"

#ifdef INFRA_LOG_ENABLED
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
#include <doctest/doctest.h>
#include <locale>

using namespace doctest;
using namespace std::string_view_literals;

int main(int argc, char** argv)
{
    std::locale::global(std::locale::classic());

#ifdef WIN32
    // make sure we can print utf8 characters in the windows console
    SetConsoleOutputCP(CP_UTF8);
#endif

#ifdef INFRA_LOG_ENABLED
    inf::Log::add_console_sink(inf::Log::Colored::On);
    inf::LogRegistration logReg("InfraTest");
    inf::Log::set_level(inf::Log::Level::Info);

    for (int i = 0; i < argc; ++i) {
        if ("-d"sv == argv[i]) {
            inf::Log::set_level(inf::Log::Level::Debug);
            break;
        }
    }
#endif

#ifdef HAVE_GDAL
    fs::path proj4DataPath = fs::u8path(argv[0]).parent_path() / "data";
    inf::gdal::Registration reg(proj4DataPath);

    CPLSetConfigOption("GDAL_DISABLE_READDIR_ON_OPEN ", "TRUE");
#endif

    doctest::Context context;
    context.applyCommandLine(argc, argv);

    return context.run();
}
