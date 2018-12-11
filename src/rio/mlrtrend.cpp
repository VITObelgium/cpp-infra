#include <iostream>

#include "infra/exception.h"
#include "infra/log.h"
#include "infra/xmldocument.h"

#include "mlrtrend.hpp"

namespace rio {

using namespace inf;


mlrtrend::mlrtrend(const std::string& config_file)
: trendmodel()
{
    _name = "mlrtrend";

    // Get filename with parameters and import as a boost property tree
    Log::info("Importing trend model config from: {}", config_file);

    auto doc = XmlDocument::load_from_file(config_file);
    if (!doc) {
        throw RuntimeError("unable to load/parse xml file {}", config_file);
    }

    auto rootEl = doc.root_node();
    if (!rootEl || rootEl.name() != "trendmodel") {
        throw RuntimeError("no <trendmodel> root element found in {}");
    }

    // Look up the class... is this the correct config file ?
    auto model_class = rootEl.child("class").trimmed_value();
    if (model_class != "mlrtrend") {
        throw RuntimeError("trend model config type does not match, expected 'mlrtrend' got '{}'", model_class);
    }
    
}

mlrtrend::~mlrtrend()
{
}

void mlrtrend::select(const std::string& aggr, boost::posix_time::ptime tstart)
{
    std::cout << "detrending mlrtrend...\n";

    return;
}

void mlrtrend::detrend(double& v, const std::string& station, const std::vector<double> proxy)
{

    std::cout << "detrending mlrtrend...\n";

    return;
}

void mlrtrend::addtrend(double& v, double& e, const std::vector<double> proxy)
{
    std::cout << "addtrend mlrtrend...\n";

    return;
}

}
