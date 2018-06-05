#include "MLP_FeedForwardModel.h"

#include "AQNetwork.h"
#include "AQNetworkProvider.h"
#include "Station.h"
#include "TimeSeries.h"
#include "data/ForecastBuffer.h"
#include "feedforwardnet.h"
#include "infra/configdocument.h"
#include "infra/log.h"
#include "infra/string.h"

#include <limits>

// define place holder for the neural network input files
static const std::string POLLUTANT_PLACEHOLDER   = "%pol%";     // placeholder for the pollutant in config file
static const std::string AGGREGATION_PLACEHOLDER = "%aggr%";    // placeholder ofr the aggregation in the config file
static const std::string STATION_PLACEHOLDER     = "%station%"; // placeholder for the station in config file
static const std::string FCHOR_PLACEHOLDER       = "%fc_hor%";  // idem for forecast horizon
static const std::string MODEL_PLACEHOLDER       = "%model%";   // idem for feature vector model

namespace opaq {

using namespace infra;

static const LogSource s_logSrc = "MLP_FeedForwardModel";

MLP_FeedForwardModel::MLP_FeedForwardModel()
: sample_size(0)
{
}

std::string MLP_FeedForwardModel::getFFNetFile(const std::string& pol_name, Aggregation::Type aggr,
    const std::string& st_name, int fc_hor)
{
    // Building filename...
    std::string fname = this->pattern;
    str::replaceInPlace(fname, POLLUTANT_PLACEHOLDER, pol_name);
    str::replaceInPlace(fname, AGGREGATION_PLACEHOLDER, Aggregation::getName(aggr));
    str::replaceInPlace(fname, STATION_PLACEHOLDER, st_name);
    str::replaceInPlace(fname, FCHOR_PLACEHOLDER, std::to_string(fc_hor));
    str::replaceInPlace(fname, MODEL_PLACEHOLDER, this->getName());

    return fname;
}

// need to create a routine which can be re-used by the OVL model without having to make use of the
// run( ) method, as this will require updating the buffers, as we want OVL to be a standalone plugin,
// we should make it independent of the output of the other models --> so run them twice !!!
double MLP_FeedForwardModel::fcValue(const Pollutant& pol, const Station& station,
    Aggregation::Type aggr, const chrono::date_time& baseTime,
    chrono::days fc_hor)
{
    // Return the neural network filename, should be implemented in the daughter class
    auto fname = getFFNetFile(pol.getName(), aggr, station.getName(), fc_hor.count());

    // Read in the network file

    try {
        auto nnet_xml = ConfigDocument::loadFromFile(fname);

        // construct the neural network object
        std::unique_ptr<nnet::feedforwardnet> net;
        try {
            net = std::make_unique<nnet::feedforwardnet>(nnet_xml.rootNode());
        } catch (const std::exception& e) {
            Log::error(s_logSrc, "Unable to construct ffnet in {} ({})", fname, e.what());
            return getNoData();
        }

        if (net->inputSize() != this->sample_size) {
            throw RunTimeException("Invalid network input size ({}) for model sample size ({})", net->inputSize(), sample_size);
        }

        auto fcTime = baseTime + fc_hor;

        // construct the input feature vector, output is single pointer value
        // not really nice, but let's leave it for the moment...
        std::vector<double> input_sample(net->inputSize());
        double* output;

        // call abstract method to generate the sample
        if (this->makeSample(input_sample.data(), station, pol, aggr, baseTime, fcTime, fc_hor)) {
            Log::error(s_logSrc, "   input sample incomplete, setting missing value");
            return getNoData();
        }

        // for ( int ii=0; ii< net->inputSize(); ++ii )
        //   std::cout << "input_sample["<<ii<<"] = " << input_sample[ii] << std::endl;

        // simulate the network
        net->sim(input_sample.data());

        // retrieve the output & reset the logtransform
        net->getOutput(&output);
        double out = exp(output[0]) - 1;

        if (std::isnan(out) || std::isinf(out)) {
            out = getNoData();
        }

        return out;
    } catch (const std::exception& e) {
        Log::error(s_logSrc, "   unable to load ffnet from: {} ({})", fname, e.what());
        return getNoData();
    }
}

/* ============================================================================

     Implementation of the model run method

     ========================================================================== */
void MLP_FeedForwardModel::run()
{
    // -- 1. initialization
    Log::debug(s_logSrc, "MLP_FeedForwardModel {} run() method called", getName());

    auto baseTime          = getBaseTime();
    Pollutant pol          = getPollutant();
    Aggregation::Type aggr = getAggregation();
    auto& net              = getAQNetworkProvider().getAQNetwork();
    ForecastBuffer* buffer = getBuffer();

    auto& stations = net.getStations();

    // -- Forecast horizon
    // forecast horizon requested by user is available in abstract model and
    // defined in the configuration file under <forecast><horizon></horizon></forecast>
    // value is given in days, but stored in the TimeInterval format, so have to get days back
    int fcHorMax = getForecastHorizon().count();

    // -- 2. loop over the stations
    for (auto& station : stations) {
        // check if we have a valid meteo id, otherwise skip the station
        if (station.getMeteoId().empty()) {
            Log::debug(s_logSrc, "Skipping station {}, no meteo id given", station.getName());
            continue;
        } else
            Log::debug(s_logSrc, "Forecasting station {}", station.getName());

        // store the output in a timeseries object
        TimeSeries<double> fc;
        fc.clear();

        for (int fc_hor = 0; fc_hor <= fcHorMax; ++fc_hor) {
            chrono::days fcHor(fc_hor);
            auto fcTime = baseTime + fcHor;
            Log::debug(s_logSrc, " -- basetime: {}, horizon: day {}, dayN is: {}", chrono::to_date_string(baseTime), fc_hor, chrono::to_date_string(fcTime));

            fc.insert(fcTime, fcValue(pol, station, aggr, baseTime, fcHor));
        }

        // now we have all the forecast values for this particular station, set the output values...
        buffer->setCurrentModel(this->getName());
        buffer->setValues(baseTime, fc, station.getName(), pol.getName(), aggr);
    }
}

/* ---------------------------------------------------------------------------------------------- */

/**
 * Computes the mean of the array, taking into account the missing values..
 * just a helper routine
 */
double MLP_FeedForwardModel::mean_missing(const std::vector<double>& list, double noData)
{
    double out    = 0;
    int dataCount = 0;

    for (auto value : list) {
        if (value != noData) {
            out += value;
            ++dataCount;
        }
    }

    return dataCount > 0 ? out / dataCount : noData;
}

/**
 * Computes the max of the array, taking into account the missing values..
 * just a helper routine
 */
double MLP_FeedForwardModel::max_missing(const std::vector<double>& list, double noData)
{
    double out = std::numeric_limits<double>::min();
    auto it    = list.begin();

    while (it != list.end()) {
        double value = *it++;
        if ((value != noData) && (value > out)) out = value;
    }
    return out > std::numeric_limits<double>::min() ? out : noData;
}

/**
 * Computes the min of the array, taking into account the missing values..
 * just a helper routine
 */
double MLP_FeedForwardModel::min_missing(const std::vector<double>& list, double noData)
{
    double out = std::numeric_limits<double>::max();
    auto it    = list.begin();

    while (it != list.end()) {
        double value = *it++;
        if ((value != noData) && (value < out)) out = value;
    }
    return out < std::numeric_limits<double>::max() ? out : noData;
}

/**
 * Simple quick 'n dirty debugging routine to print out the parameters
 */
void MLP_FeedForwardModel::printPar(const std::string& title, const std::vector<double>& x)
{
    std::stringstream ss;
    ss << title;
    for (auto value : x) {
        ss << " " << value;
    }

    Log::debug(s_logSrc, ss.str());
}

} /* namespace OPAQ */
