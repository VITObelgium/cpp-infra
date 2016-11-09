#include "OVL.h"

#include "MLP_FeedForwardModel.h"
#include "feedforwardnet.h"

#include "Engine.h"
#include "Station.h"
#include "AQNetwork.h"
#include "PluginRegistration.h"
#include "tools/XmlTools.h"

#include <cmath>
#include <fstream>

namespace opaq
{

using namespace chrono_literals;

OVL::OVL()
: Model("OVL")
, _componentMgr(nullptr)
, _output_raw(false)
, _debug_output(false)
{
}

std::string OVL::name()
{
    return "ovl";
}

/**
 * Helper function to compute the weighted exponetial weights. This is based upon
 *
 * \sum_{i=0}^{N-1} r^n = \frac{1-r^N}{1-r}
 *
 * and is used in mode 2 of the RTC where the hindcast errors are weighted exponentially, given larger
 * weight to the more recent errors.
 *
 * The shape of the exponential
 * weight is determined by the parameter 'param' (> 0). For  smaller values this
 * parameter will give more weight to the recent errors, for param -> inf this will
 * distribute the weight more equally across the hind cast. The limit of param -> infinity is
 * mode 1, the simple average... THe table below gives some indication of the weight of the
 * first and second most recent errors :
 *
 *      param | weights...
 *      ------|--------------------------
 *        0   | 1.00 0.00 0.00 0.00 0.00
 *        1   | 0.50 0.25 0.13 0.06 0.03
 *        2   | 0.33 0.22 0.15 0.10 0.07
 *        3   | 0.25 0.18 0.14 0.11 0.08
 *
 *  \param p is the parameter p for the exponential (given in the XML station config)
 *  \param i is the index, where i = 0 means yesterday (the first index in the array of
 *           hind cast errors
 *  \param n is the length of the hindcast window
 */
double _wexp(int i, int n, int p)
{
    double lambda = static_cast<double>(p) / (1. + static_cast<double>(p));
    return (1. - lambda) * std::pow(lambda, i) / (1. - std::pow(lambda, n));
}

void OVL::configure(TiXmlElement* cnf, const std::string& componentName, IEngine& engine)
{
    setName(componentName);

    // here the actual station configuation & RTC configuration should be read, the individual models
    // are already defined... in there respective plugins...
    _conf.clear();
    _componentMgr = &engine.componentManager();

    try
    {

        // read the mode for which the corresponding tune should be taken
        _tune_mode = XmlTools::getText(cnf, "tune_mode");

        // read missing value for this model
        setNoData(atoi(XmlTools::getText(cnf, "missing_value").c_str()));

        // read hind cast period
        _hindcast = chrono::days(atoi(XmlTools::getText(cnf, "hindcast").c_str()));

        // parse the tunes database & run configuration for OVL
        parseTuneList(cnf->FirstChildElement("tunes"));
    }
    catch (BadConfigurationException& err)
    {
        throw BadConfigurationException(err.what());
    }

    // get output mode
    try
    {
        std::string s = XmlTools::getText(cnf, "output_raw");
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (!s.compare("enable") || !s.compare("true") || !s.compare("yes")) _output_raw = true;
    }
    catch (...)
    {
        _output_raw = false;
    }

    // activate debugging output, hard coded for now
    // get output mode
    try
    {
        std::string s = XmlTools::getText(cnf, "_debug_output");
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (!s.compare("enable") || !s.compare("true") || !s.compare("yes")) _debug_output = true;
    }
    catch (...)
    {
        _debug_output = false;
    }
}

void OVL::parseTuneElement(TiXmlElement* tuneEl)
{

    std::string tunePol, tuneAggr, tuneMode;
    if (tuneEl->QueryStringAttribute("pollutant", &tunePol) != TIXML_SUCCESS) throw BadConfigurationException("pollutant not found in tune");
    if (tuneEl->QueryStringAttribute("aggr", &tuneAggr) != TIXML_SUCCESS) throw BadConfigurationException("aggr not found in tune");
    if (tuneEl->QueryStringAttribute("mode", &tuneMode) != TIXML_SUCCESS) throw BadConfigurationException("mode not found in tune");

    TiXmlElement* stEl = tuneEl->FirstChildElement("station");
    while (stEl)
    {
        std::string stName = stEl->Attribute("name");

        TiXmlElement* modelEl = stEl->FirstChildElement("model");
        while (modelEl)
        {
            OVL::StationConfig c;
            int fc_hor;

            modelEl->QueryIntAttribute("fc_hor", &fc_hor);
            modelEl->QueryIntAttribute("rtc_mode", &(c.rtc_mode));
            modelEl->QueryIntAttribute("rtc_param", &(c.rtc_param));
            c.model_name = modelEl->GetText();

            // inserting using list initializer instead of std::pair()... doesnt work for replacing the std::make_tuple command though
            _conf.insert({std::make_tuple(tunePol, Aggregation::fromString(tuneAggr), tuneMode, stName, fc_hor), c});

            modelEl = modelEl->NextSiblingElement("model");
        }

        stEl = stEl->NextSiblingElement("station");
    } // loop over the stations
}

void OVL::parseTuneList(TiXmlElement* lst)
{

    TiXmlDocument refDoc;
    TiXmlElement* tuneEl = lst->FirstChildElement("tune");
    while (tuneEl)
    {
        std::string ref;
        if (tuneEl->QueryStringAttribute("ref", &ref) == TIXML_SUCCESS) {
            // ref attribute found
            if (FileTools::exists(ref)) {
                // file found
                if (!refDoc.LoadFile(ref)) {
                    throw ElementNotFoundException("Failed to load file in ref attribute: {}", ref);
                }
                TiXmlElement* fileElement = refDoc.FirstChildElement("tune");
                if (!fileElement) {
                    throw ElementNotFoundException("File in ref attribute ({}) does not have 'tune' as root element", ref);
                }
                parseTuneElement(fileElement);
            }
            else
            {
                throw ElementNotFoundException("File in ref attribute '{}' not found.", ref);
            }
        }
        else
        {
            parseTuneElement(tuneEl);
        }

        tuneEl = tuneEl->NextSiblingElement("tune");
    } // loop over the pollutants

    return;
}

/* ============================================================================

     Implementation of the model run method

     ========================================================================== */
void OVL::run()
{

    // -- 1. initialization
    _logger->debug("OVL " + getName() + " run() method called");

    auto baseTime          = getBaseTime();
    Pollutant pol          = getPollutant();
    Aggregation::Type aggr = getAggregation();
    ForecastBuffer* buffer = getBuffer();

    auto& stations = getAQNetworkProvider().getAQNetwork().getStations();

    // -- Forecast horizon
    // forecast horizon requested by user is available in abstract model and
    // defined in the configuration file under <forecast><horizon></horizon></forecast>
    // value is given in days, but stored in the TimeInterval format, so have to get days back
    int fcHorMax = getForecastHorizon().count();

    // some debugging output ?
    std::ofstream fs;
    if (_debug_output) {
        fs.open(std::string("ovl_debug") + "_" + pol.getName() + "_" + Aggregation::getName(aggr) + "_" + chrono::to_date_string(baseTime) + ".txt");
    }

    // -- 2. loop over the stations, the C++ 11 way :)
    for (auto& station : stations)
    {

        // check if we have a valid meteo id, otherwise skip the station
        if (station.getMeteoId().length() == 0) {
            _logger->trace("Skipping station " + station.getName() + ", no meteo id given");
            continue;
        }
        else
            _logger->trace("Forecasting station " + station.getName());

        if (_debug_output) fs << "[STATION] " << station.getName() << " - " << chrono::to_date_string(baseTime) << std::endl;

        // store the output in a time series object
        TimeSeries<double> fc;

        for (int fc_hor = 0; fc_hor <= fcHorMax; fc_hor++)
        {

            auto fcHor  = chrono::days(fc_hor);
            auto fcTime = baseTime + fcHor;

            if (_debug_output) fs << "\t[DAY+" << fc_hor << "]" << std::endl;

            // lookup the station configuration
            auto stIt = _conf.find(std::make_tuple(pol.getName(), aggr, _tune_mode, station.getName(), fc_hor));
            if (stIt == _conf.end()) {
                // no configuration for this station, returning -9999 or something
                _logger->warn("Model configuration not found for " + pol.getName() + ", st = " + station.getName() + ", skipping...");
                fc.insert(fcTime, getNoData());
                continue;
            }

            // get a pointer to the station configuration
            StationConfig* cf = &(stIt->second);

            // get the correct model plugin, we don't have to destroy it as there is only one instance of each component,
            // configuration via the setters...
            assert(_componentMgr);
            auto& model = _componentMgr->getComponent<MLP_FeedForwardModel>(cf->model_name);

            // set ins and outs for the model here...
            // this is in fact a small engine...
            model.setBaseTime(baseTime);
            model.setPollutant(pol);
            model.setAQNetworkProvider(getAQNetworkProvider());
            model.setForecastHorizon(getForecastHorizon());
            model.setInputProvider(getInputProvider());
            model.setMeteoProvider(getMeteoProvider());
            model.setBuffer(getBuffer());

            // run the model
            double out = model.fcValue(pol, station, aggr, baseTime, fcHor);

            if (_debug_output) {
                fs << "\t\tNN MODEL    : " << model.getName() << std::endl;
                fs << "\t\tNN FORECAST : " << out << std::endl;
                fs << "\t\tRTC MODE    : " << cf->rtc_mode << std::endl;
            }

            // if we do not have all the model components loaded in OPAQ that OVL is using, we have
            // to store the output otherwise the RTC messes up
            if (_output_raw) {
                // now we have all the forecast values for this particular station, set the output values...
                TimeSeries<double> raw_fc;
                raw_fc.clear();
                raw_fc.insert(fcTime, out);
                buffer->setCurrentModel(model.getName());
                buffer->setValues(baseTime, raw_fc, station.getName(), pol.getName(), aggr);
            }

            // now handle the RTC, if the mode is larger than 0, otherwise we already have out output !!!
            // only to this if the output value of the model is not missing...
            if (cf->rtc_mode > 0 && fabs(out - model.getNoData()) > 1.e-6) {

                if (_debug_output) {
                    fs << "\t\tRTC PARAM   : " << cf->rtc_param << std::endl;
                }

                // get the hind cast for the forecast times between yesterday & the hindcast
                auto t1 = baseTime - _hindcast;
                auto t2 = baseTime - 1_d;

                buffer->setCurrentModel(model.getName());
                TimeSeries<double> fc_hindcast = buffer->getForecastValues(fcHor, t1, t2, station.getName(), pol.getName(), aggr);

                if (_debug_output) {
                    fs << "\t\tHINDCAST : " << std::endl;
                    for (unsigned int i = 0; i < fc_hindcast.size(); i++)
                        fs << "\t\t"
                           << chrono::to_date_string(fc_hindcast.datetime(i)) << "\t"
                           << fc_hindcast.value(i) << std::endl;
                }

                // get the observed values from the input provider
                // we could also implement it in such way to get them back from the forecast buffer...
                // here a user should simply make sure we have enough data available in the data provider...
                auto obs_hindcast = getInputProvider().getValues(t1, t2, station.getName(), pol.getName(), aggr);

                if (_debug_output) {
                    fs << "\t\tOBSERVATIONS : " << std::endl;
                    for (unsigned int i = 0; i < fc_hindcast.size(); i++)
                        fs << "\t\t"
                           << chrono::to_date_string(obs_hindcast.datetime(i)) << "\t"
                           << obs_hindcast.value(i) << std::endl;
                }

                // check if the timeseries are consistent !
                if (!fc_hindcast.isConsistent(obs_hindcast))
                    throw RunTimeException("forecast & hindcast timeseries are not consistent...");

                double fc_err = 0.;
                // run the real time correction scheme, let's do it this way for the moment,
                // later on we can add a separate class or even plugin for this...
                switch (cf->rtc_mode)
                {
                case 1:
                { // compute the average error in the hind cast period
                    int n = 0;
                    for (unsigned int i = 0; i < fc_hindcast.size(); i++)
                    {
                        if (fabs(fc_hindcast.value(i) - buffer->getNoData()) > 1.e-6 &&
                            fabs(obs_hindcast.value(i) - getInputProvider().getNoData()) > 1.e-6) {

                            if (_debug_output) {
                                fs << "\t\tERROR : " << chrono::to_date_string(fc_hindcast.datetime(i)) << "\t" << fc_hindcast.value(i) - obs_hindcast.value(i) << std::endl;
                            }

                            fc_err += (fc_hindcast.value(i) - obs_hindcast.value(i));

                            n++;
                        }
                    }
                    if (n > 0) fc_err /= n;

                    if (_debug_output) fs << "\t\tFINAL ERROR (mode 1) : " << fc_err << std::endl;
                }
                break;

                case 2:
                { // compute a weighted error in the hindcast period
                    // TODO here we don't take the weights of the missing values in either
                    //      obs / fc arrays into account... maybe a fix for later (was never so in OVL)
                    for (unsigned int i = 0; i < fc_hindcast.size(); i++)
                    {
                        if (fabs(fc_hindcast.value(i) - buffer->getNoData()) > 1.e-6 &&
                            fabs(obs_hindcast.value(i) - getInputProvider().getNoData()) > 1.e-6) {

                            // BUGFIX : have to turn the order of i around... !!
                            const auto fc_hindcast_size = static_cast<int>(fc_hindcast.size());
                            double w = _wexp(fc_hindcast_size - i - 1, fc_hindcast_size, cf->rtc_param);

                            if (_debug_output) {
                                fs << "\t\tERROR : " << chrono::to_date_string(fc_hindcast.datetime(i)) << "\t"
                                   << fc_hindcast.value(i) - obs_hindcast.value(i) << "\t"
                                   << "WEIGHT= " << w << std::endl;
                            }

                            fc_err += w * (fc_hindcast.value(i) - obs_hindcast.value(i));
                        }
                    }

                    if (_debug_output) fs << "\t\tFINAL ERROR (mode 2) : " << fc_err << std::endl;
                }
                break;

                default:
                    throw BadConfigurationException("Invalid real time correction mode : " + std::to_string(cf->rtc_mode));
                    break;
                }

                // perform correction, of out is not missing
                out = out - fc_err;

                if (_debug_output) fs << "\tCORRECTED FORECAST : " << out << std::endl;
            }

            // insert the final forecast...
            fc.insert(fcTime, out);
        }

        // now we have all the forecast values for this particular station, set the output values...
        buffer->setCurrentModel(getName());
        buffer->setValues(baseTime, fc, station.getName(), pol.getName(), aggr);

    } // loop over the stations
}

OPAQ_REGISTER_STATIC_PLUGIN(OVL)

} /* namespace OPAQ */
