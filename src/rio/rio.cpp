// -*-c++-*-
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#include <boost/date_time.hpp>

#include <Eigen/Core>
#include <Eigen/Dense>

#include "rio.hpp"

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

    try {
        std::string setup_file = get_envvar("RIO_SETUP_FILE");
        if (setup_file.empty()) setup_file = "rio_setup.xml";

        rio::config cf;
        cf.parse_command_line(argc, argv);
        cf.parse_setup_file(setup_file);
        std::cout << cf;

        // read network
        std::cout << "[Network]" << std::endl;
        std::shared_ptr<rio::network> net = std::make_shared<rio::network>(cf.stationConfig());
        if (cf.debug()) std::cout << *net;

        // read grid
        std::cout << "[Mapping grid]" << std::endl;
        std::shared_ptr<rio::grid> grid = std::make_shared<rio::grid>(cf.grid(), cf.gridConfig());
        if (cf.debug()) std::cout << *grid;

        // the observation database
        std::cout << "[Observation handler]" << std::endl;
        std::unique_ptr<rio::obshandler> dbq = std::make_unique<rio::dbqfile>(cf.obsConfig());
        dbq->setNetwork(net);

        // the interpolation model
        std::cout << "[Interpolation model]" << std::endl;
        std::unique_ptr<rio::mapper> model;
        if (boost::iequals(cf.ipol_class(), "idw_ipol")) {
            std::cout << " Creating IDW mapping model..." << std::endl;
            model = std::make_unique<rio::idw_ipol>(cf, net);

        } else if (boost::iequals(cf.ipol_class(), "krige_ipol")) {
            std::cout << " Creating Ordinary Kriging mapping model..." << std::endl;
            model = std::make_unique<rio::krige_ipol>(cf, net);

        } else if (boost::iequals(cf.ipol_class(), "rio_ipol")) {
            std::cout << " Creating Detrended Kriging (RIO) mapping model..." << std::endl;
            model = std::make_unique<rio::rio_ipol>(cf, net);

        } else {
            std::cerr << "*** unknown mapping model type : " << cf.ipol_class() << std::endl;
            return EXIT_FAILURE;
        }

        // Setup the requested output handlers
        std::cout << "[Output handlers]" << std::endl;
        rio::output out(cf.outputConfig(), cf.req_output());
        out.init(cf, net, grid);

        // Prepare datastructures
        std::map<std::string, double> obs;
        Eigen::VectorXd values, uncert;
        values.resize(grid->size());
        uncert.resize(grid->size());

        // some counters
        unsigned int nmaps = 0;

        // start timeloop
        std::cout << "Starting timeloop..." << std::endl;
        ptime curr_time = cf.start_time();
        while (curr_time <= cf.stop_time()) {
            std::cout << "Interpolating " << curr_time << std::endl;

            // update the current_time and day pattern
            rio::parser::get()->add_pattern("%timestamp%", boost::posix_time::to_iso_string(curr_time));
            rio::parser::get()->add_pattern("%date%", boost::gregorian::to_iso_string(curr_time.date()));

            dbq->get(obs, curr_time, cf.pol(), cf.aggr());

            // TODO : build this into the mappers
            if (obs.size() >= 5) {
                model->run(values, uncert, curr_time, obs, grid);
                nmaps++;
            } else {
                std::cout << "+++ warning, less than 5 observations, skipping\n";
                values.fill(-9999.);
                uncert.fill(-9999.);
            }
            out.write(curr_time, obs, values, uncert);

            curr_time += cf.tstep();
        }

        // close each output handler
        out.close();

        // Starting online postprocessing ... some things are approximations (e.g. online percentiles)
        // For accurate results best compute offline postproessing
        std::cout << "Online postprocessing" << std::endl;

        // add calculate the averages, etc..
        // add streaming percentiles via: https://github.com/sengelha/streaming-percentiles-cpp
        // however --> for the high percentiles this might not be suitable... ??

        // Done !
        std::cout << "Number of maps produced : " << nmaps << "\n";
        std::cout << "\n";
        std::cout << "All done. Have a nice day :)" << std::endl;

        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
