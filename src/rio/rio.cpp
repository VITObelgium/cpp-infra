// -*-c++-*-
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fmt/ostream.h>
#include <iostream>
#include <memory>

#include <boost/date_time.hpp>

#include <Eigen/Core>
#include <Eigen/Dense>

#include "infra/gdal.h"
#include "infra/log.h"
#include "modelrunner.hpp"
#include "rio.hpp"

using inf::Log;
using namespace boost::gregorian;
using namespace boost::posix_time;

// little helper routine to retrieve environment variable
std::string get_envvar(std::string const& key)
{
    std::string retval = "";
    char* val          = getenv(key.c_str());
    if (val != NULL) retval = val;
    return retval;
}

int main(int argc, char* argv[])
{
    std::cout << "+----------------------------------------------------+\n";
    std::cout << "|                                                    |\n";
    std::cout << "|  RIO air quality mapping model           (((       |\n";
    std::cout << "|  Version 5.0                            (. .)      |\n";
    std::cout << "|  (c) VITO/VMM 2004-2018               <(( v ))>    |\n";
    std::cout << "|                                          | |       |\n";
    std::cout << "+------------------------------------------m-m-------+" << std::endl;

    Log::add_console_sink(Log::Colored::On);
    inf::LogRegistration logging("rio");

    try {
        
        Log::set_level(Log::Level::Info);
        Log::set_pattern("%v");

        inf::gdal::Registration gdalReg;

        std::string setup_file = get_envvar("RIO_SETUP_FILE");
        if (setup_file.empty()) setup_file = "rio_setup.xml";

        rio::config cf;
        cf.parse_command_line(argc, argv);
        cf.parse_setup_file(setup_file);
        Log::info("{}", cf);

        if (cf.debug()) {
            Log::set_level(Log::Level::Debug);
        }

        auto runInfo = run_model(cf);

        // Done !
        Log::info("Number of maps produced: {}\n", runInfo.mapsProcessed);
        Log::info("All done. Have a nice day :)");

        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        Log::error(e.what());
        return EXIT_FAILURE;
    }
}
