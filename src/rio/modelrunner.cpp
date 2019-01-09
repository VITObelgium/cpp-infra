#include "modelrunner.hpp"
#include "infra/cast.h"
#include "infra/exception.h"
#include "infra/log.h"
#include "infra/string.h"
#include "rio.hpp"

#include <fmt/ostream.h>

namespace rio {

using namespace inf;

void run_model(const config& cf, std::function<bool(int)> progressCb)
{
    // Setup the requested output handlers
    Log::info("[Output handlers]");
    rio::output out(cf.outputConfig(), cf.req_output());

    run_model(cf, out, progressCb);
}
void run_model(const config& cf, output& output, std::function<bool(int)> progressCb)
{
    // read network
    Log::info("[Network]");
    auto net = std::make_shared<rio::network>(cf.stationConfig());
    Log::debug("{}", *net);

    // read grid
    Log::info("[Mapping grid]");
    auto grid = std::make_shared<rio::grid>(cf.grid(), cf.gridConfig());
    Log::debug("{}", *grid);

    // the observation database
    Log::info("[Observation handler]");
    auto dbq = std::make_unique<rio::dbqfile>(cf.obsConfig());
    dbq->setNetwork(net);

    // the interpolation model
    Log::info("[Interpolation model]");
    std::unique_ptr<rio::mapper> model;
    if (str::iequals(cf.ipol_class(), "idw_ipol")) {
        Log::info("Creating IDW mapping model...");
        model = std::make_unique<rio::idw_ipol>(cf, net);
    } else if (str::iequals(cf.ipol_class(), "krige_ipol")) {
        Log::info("Creating Ordinary Kriging mapping model...");
        model = std::make_unique<rio::krige_ipol>(cf, net);
    } else if (str::iequals(cf.ipol_class(), "rio_ipol")) {
        Log::info("Creating Detrended Kriging (RIO) mapping model...");
        model = std::make_unique<rio::rio_ipol>(cf, net);
    } else {
        throw RuntimeError("unknown mapping model type : {}", cf.ipol_class());
    }

    output.init(cf, net, grid);

    // Prepare datastructures
    Eigen::VectorXd values, uncert;
    values.resize(grid->size());
    uncert.resize(grid->size());

    // some counters
    unsigned int nmaps = 0;

    // Start timeloop, internally the querying works with start times of the time interval
    // to which the value applies.
    Log::info("Starting timeloop...");
    auto curr_time  = cf.start_time();
    bool keepGoing  = true;
    int currentStep = 0;
    int timeSteps   = truncate<int>((cf.stop_time() - cf.start_time()).total_seconds() / cf.tstep().total_seconds());

    while (curr_time <= cf.stop_time() && keepGoing) {
        Log::info("Interpolating {}", curr_time);

        // update the current_time and day pattern
        rio::parser::get()->add_pattern("%timestamp%", boost::posix_time::to_iso_string(curr_time));
        rio::parser::get()->add_pattern("%date%", boost::gregorian::to_iso_string(curr_time.date()));

        auto obs = dbq->get(curr_time, cf.pol(), cf.aggr());

        // TODO : build this into the mappers
        if (obs.size() >= 5) {
            model->run(values, uncert, curr_time, obs, grid);
            nmaps++;
        } else {
            Log::warn("less than 5 observations, skipping");
            values.fill(-9999.);
            uncert.fill(-9999.);
        }

        // always write output, even though the values are all missing...
        output.write(curr_time, obs, values, uncert);

        curr_time += cf.tstep();

        if (progressCb) {
            float progress = currentStep++ / float(timeSteps);
            keepGoing      = progressCb(truncate<int>(progress * 100));
        }
    }

    // close each output handler
    output.close();

    // Starting online postprocessing ... some things are approximations (e.g. online percentiles)
    // For accurate results best compute offline postproessing
    Log::info("Online postprocessing");

    // add calculate the averages, etc..
    // add streaming percentiles via: https://github.com/sengelha/streaming-percentiles-cpp
    // however --> for the high percentiles this might not be suitable... ??
}
}
