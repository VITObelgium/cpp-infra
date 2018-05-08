#include "AsciiForecastWriter.h"

#include "AQNetwork.h"
#include "PluginRegistration.h"

#include "infra/configdocument.h"
#include "infra/string.h"

#include <iterator>
#include <vector>

namespace opaq {

using namespace infra;

const std::string AsciiForecastWriter::BASETIME_PLACEHOLDER    = "%basetime%";
const std::string AsciiForecastWriter::POLLUTANT_PLACEHOLDER   = "%pol%";
const std::string AsciiForecastWriter::AGGREGATION_PLACEHOLDER = "%aggr%";

AsciiForecastWriter::AsciiForecastWriter()
: _logger("AsciiForecastWriter")
, _enable_fields(false)
, _sepchar('\t')
, _fctime_full(true)
, _full_output(true)
{
}

AsciiForecastWriter::~AsciiForecastWriter()
{
}

std::string AsciiForecastWriter::name()
{
    return "asciiforecastwriter";
}

void AsciiForecastWriter::configure(const ConfigNode& configuration, const std::string& componentName, IEngine&)
{
    setName(componentName);

    // parse filename
    auto fileEl = configuration.child("filename");
    if (!fileEl) {
        throw BadConfigurationException("filename element not found");
    }

    _filename = fileEl.value();

    auto model_list = configuration.child("models").value();
    _models         = str::split(model_list,  StringTools::tokenize(model_list, ",;:|/ \t", 7);

    _title = configuration.child("title").value();
    _title = configuration.child("header").value();

    // get output mode
    _enable_fields = false;
    auto fields    = str::lowercase(configuration.child("fields").value());
    _enable_fields = fields == "enable" || fields == "true" || fields == "yes";

    // -- get separation character
    auto sepchar = str::lowercase(configuration.child("sepchar").value());
    if (!sepchar.empty()) {
        if (sepchar == "tab") {
            _sepchar = '\t';
        } else if (sepchar == "space") {
            _sepchar = ' ';
        } else {
            _sepchar = sepchar[0]; //get the first character
        }
    }

    // get output mode for fcTime
    auto fctime  = str::lowercase(configuration.child("fctime").value());
    _fctime_full = fctime.empty() || fctime == "full";

    auto fullOutput = str::lowercase(configuration.child("full_output").value());
    _full_output    = fullOutput.empty() || fullOutput == "enable" || fields == "true" || fields == "yes";
}

void AsciiForecastWriter::write(const Pollutant& pol, Aggregation::Type aggr, const chrono::date_time& baseTime)
{
    std::string fname = _filename;
    std::string head  = _header;

    if (!getBuffer()) throw RunTimeException("No databuffer set");
    if (!getAQNetworkProvider()) throw RunTimeException("No AQ network set");

    // -- get network & stations & maximum forecast horizon
    auto& stations = getAQNetworkProvider()->getAQNetwork().getStations();
    int fcHorMax   = getForecastHorizon().count();

    // -- translate the filename
    str::replaceInPlace(fname, POLLUTANT_PLACEHOLDER, pol.getName());
    str::replaceInPlace(fname, AGGREGATION_PLACEHOLDER, Aggregation::getName(aggr));
    str::replaceInPlace(fname, BASETIME_PLACEHOLDER, chrono::to_date_string(baseTime));

    // -- translate the header
    str::replaceInPlace(head, POLLUTANT_PLACEHOLDER, pol.getName());
    str::replaceInPlace(head, AGGREGATION_PLACEHOLDER, Aggregation::getName(aggr));
    str::replaceInPlace(head, BASETIME_PLACEHOLDER, chrono::to_date_string(baseTime));

    // ========================================================================
    // initialization
    // ========================================================================
    _logger->info("Writing output file " + fname);

    FILE* fp = fopen(fname.c_str(), "w");
    if (!fp) {
        throw RunTimeException("Unable to open output file " + fname);
    }

    // -- print header
    if (_title.size() != 0) fprintf(fp, "# %s\n", _title.c_str());
    if (head.size() != 0) fprintf(fp, "# %s\n", head.c_str());
    if (_enable_fields) {
        if (_fctime_full)
            fprintf(fp, "BASETIME%cSTATION%cFCTIME", _sepchar, _sepchar);
        else
            fprintf(fp, "BASETIME%cSTATION%cFCHOR", _sepchar, _sepchar);
    }

    // -- get the results for the models for this baseTime/fcTime combination
    auto modelNames = getBuffer()->getModelNames(pol.getName(), aggr);

    // -- determine the indices of the requested models
    std::vector<unsigned int> idx;
    for (auto& m : _models) {
        size_t i = std::find(modelNames.begin(), modelNames.end(), m) - modelNames.begin(); // look up index of this model in the list of available models
        if (i < modelNames.size()) {
            idx.push_back(static_cast<unsigned int>(i));
        }
    }
    // -- no index defined, dump all models
    if (idx.empty()) {
        for (unsigned int i = 0; i < modelNames.size(); i++)
            idx.push_back(i);
    }

    // -- if we have to write the fields
    if (_enable_fields) {
        for (auto ii : idx)
            fprintf(fp, "%c%s", _sepchar, modelNames[ii].c_str());
        fprintf(fp, "\n");
        fflush(fp);
    }

    // ========================================================================
    // loop over stations and produce the output
    // ========================================================================
    for (auto& station : stations) {
        if (!_full_output) {
            // check whether this station acutally measures the pollutant requested,
            // if not then we skip this station
            if (!station.measuresPollutant(pol)) {
                continue;
            }
        }

        // loop over the different forecast horizons
        for (int fc_hor = 0; fc_hor <= fcHorMax; ++fc_hor) {
            auto fcHor  = chrono::days(fc_hor);
            auto fcTime = baseTime + fcHor;

            if (_fctime_full) {
                fprintf(fp, "%s%c%s%c%s", chrono::to_date_string(baseTime).c_str(), _sepchar, station.getName().c_str(), _sepchar, chrono::to_date_string(fcTime).c_str());
            } else {
                fprintf(fp, "%s%c%s%c%d", chrono::to_date_string(baseTime).c_str(), _sepchar, station.getName().c_str(), _sepchar, fc_hor);
            }

            try {
                auto modelVals = getBuffer()->getModelValues(baseTime, fcHor, station.getName(), pol.getName(), aggr);
                if (modelVals.size() != modelNames.size())
                    throw RunTimeException("data size doesn't match the number of models...");

                for (auto ii : idx)
                    fprintf(fp, "%c%.6f", _sepchar, modelVals[ii]);
                fprintf(fp, "\n");
            } catch (const NotAvailableException&) {
                for (size_t i = 0; i < idx.size(); ++i)
                    fprintf(fp, "%c%f", _sepchar, getBuffer()->getNoData());
                fprintf(fp, "\n");
            }

        } // loop over forecast horizons

    } // loop over stations

    fclose(fp);
}

OPAQ_REGISTER_STATIC_PLUGIN(AsciiForecastWriter)
}
