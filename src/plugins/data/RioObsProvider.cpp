/*
 * RioObsProvider.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "RioObsProvider.h"
#include "AQNetworkProvider.h"
#include "ObsParser.h"

#include "infra/configdocument.h"
#include "infra/log.h"
#include "infra/string.h"

namespace opaq {

using namespace infra;
using namespace chrono_literals;
using namespace std::chrono_literals;

static const LogSource s_logSrc("RioObsProvider");
static const char* s_pollutantPlaceholder = "%pol%";

RioObsProvider::RioObsProvider()
: _noData(-9999)      // RIO observations use -9999 as nodata placeholder
, _timeResolution(1h) // RIO observations have hourly resolution, per default, can be overwritten !
, _configured(false)
, _nvalues(24)
{
}

std::string RioObsProvider::name()
{
    return "rioobsprovider";
}

void RioObsProvider::configure(const ConfigNode& configuration, const std::string& componentName, IEngine&)
{
    setName(componentName);

    _pattern = std::string(configuration.child("file_pattern").value());
    auto res = configuration.child("resolution").value<int>().value_or(0);
    if (res != 0) {
        _timeResolution = std::chrono::duration_cast<std::chrono::hours>(std::chrono::minutes(res));
        _nvalues        = (60 * 24) / res;
    }

    _buffer.clear();
    _configured = true;
}

std::chrono::hours RioObsProvider::getTimeResolution()
{
    return _timeResolution;
}

double RioObsProvider::getNoData()
{
    return _noData;
}

TimeSeries<double> RioObsProvider::getValues(const chrono::date_time& t1, const chrono::date_time& t2,
    const std::string& stationId, const std::string& pollutantId,
    Aggregation::Type aggr)
{
    // do some checks
    if (!_configured) throw RuntimeError("Not fully configured");
    if (t1 >= t2) throw InvalidArgument("First date is after the second... hmmm");

    // get pointer to buffered data
    auto& data = _getTimeSeries(pollutantId, stationId, aggr);
    if (data.isEmpty()) {
        return data;
    }

    // TODO to be safe better round down the t1 and t2 to the interval of the timestep, but is not really high priority now..
    // was originally like this in Stijn VL 's code...

    std::chrono::hours step = (aggr == Aggregation::None) ? 1h : 1_d;

    //copy the data to the output time series and insert missing values if still needed (we cannot rely on i)
    data.setNoData(_noData);
    return data.select(t1, t2, step);
}

// this returns a reference to where the full array of data is stored for this
// particular combination of pollutant, station and aggregation
TimeSeries<double>& RioObsProvider::_getTimeSeries(const std::string& pollutant,
    const std::string& station,
    Aggregation::Type aggr)
{
    // first find the pollutant in the map, if we didn't find it, read the file,
    // this should parse the whole set of stations & aggregation times, so no need
    // to re read the file afterwards, only return 0 when data is not found...
    if (_buffer.find(pollutant) == _buffer.end()) {
        // not found: read data file..
        readFile(pollutant);
    }

    return _buffer[pollutant][aggr][station];
}

// parse the file and read in the pollutant //
// no need to specify the aggregation time since we load the whole file in memory and build the
// map in one go...
void RioObsProvider::readFile(const std::string& pollutant)
{
    // create file name & open file stream
    std::string filename = _pattern;
    str::replaceInPlace(filename, s_pollutantPlaceholder, pollutant);

    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        Log::warn(s_logSrc, "Failed to open file: {}", filename);
        return;
    }

    assert(_aqNetworkProvider);
    _buffer[pollutant] = readObservationsFile(file, _aqNetworkProvider->getAQNetwork(), _nvalues, _timeResolution);
}
}
