/*
 * RioObsProvider.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "RioObsProvider.h"
#include "ObsParser.h"
#include "PluginRegistration.h"

#include "tools/StringTools.h"

#include <tinyxml.h>

namespace OPAQ
{

using namespace chrono_literals;
using namespace std::chrono_literals;

const std::string RioObsProvider::POLLUTANT_PLACEHOLDER = "%pol%";

RioObsProvider::RioObsProvider()
: _logger("RioObsProvider")
, _noData(-9999)                      // RIO observations use -9999 as nodata placeholder
, _timeResolution(1h) // RIO observations have hourly resolution, per default, can be overwritten !
, _configured(false)
, _nvalues(24)
{
}

std::string RioObsProvider::name()
{
    return "RioObsProvider";
}

// OPAQ::Component methods
void RioObsProvider::configure(TiXmlElement* cnf, const std::string& componentName, IEngine&)
{
    setName(componentName);

    // -- parse data file pattern
    TiXmlElement* patternElement = cnf->FirstChildElement("file_pattern");
    if (!patternElement)
        throw BadConfigurationException("file_pattern element not found");
    _pattern = patternElement->GetText();

    // -- parse time resolution in minutes, if given...
    TiXmlElement* resEl = cnf->FirstChildElement("resolution");
    if (resEl) {
        int res         = atoi(resEl->GetText());
        _timeResolution = std::chrono::duration_cast<std::chrono::hours>(std::chrono::minutes(res));
        _nvalues        = (60 * 24) / res;
    }

    // -- clear buffer
    _buffer.clear();

    // -- we have a configuration
    _configured = true;
}

// OPAQ::DataProvider methods
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
    if (!_configured) throw RunTimeException("Not fully configured");
    if (t1 >= t2) throw InvalidArgumentsException("First date is after the second... hmmm");

    // get pointer to buffered data
    auto* data = _getTimeSeries(pollutantId, stationId, aggr);
    if (!data)
    {
        return OPAQ::TimeSeries<double>();
    }

    // TODO to be safe better round down the t1 and t2 to the interval of the timestep, but is not really high priority now..
    // was originally like this in Stijn VL 's code...

    std::chrono::hours step = (aggr == OPAQ::Aggregation::None) ? 1h : 1_d;

    //copy the data to the output time series and insert missing values if still needed (we cannot rely on i)
    data->setNoData(_noData);
    return data->select(t1, t2, step);
}

/*
 *  old data retrieval for mapping
 *
std::vector<double> RioObsProvider::getValues(const std::string & pollutant,
		const TimeInterval & offset, const ForecastHorizon & forecastHorizon) {
	std::vector<double> out;
	AQNetwork * net = _aqNet->getAQNetwork();
	Pollutant * pol = Config::PollutantManager::getInstance()->find(pollutant);
	std::vector<Station *> * stations = &(net->getStations());
	std::vector<Station *>::iterator it = stations->begin();
	while (it != stations->end()) {
		Station * station = *it++;
		if (AQNetworkTools::stationHasPollutant(station, *pol)) {
			// station has data for the given pollutant
			out.push_back(
					getValues(offset, offset, pollutant, station->getName(),
							forecastHorizon).front());
		} else {
			// station doesn't have data for the given pollutant
			out.push_back(_noData);
		}
	}
	return out;
}
*/

// this returns a reference to where the full array of data is stored for this
// particular combination of pollutant, station and aggregation
TimeSeries<double>* RioObsProvider::_getTimeSeries(const std::string& pollutant,
                                                   const std::string& station,
                                                   Aggregation::Type aggr)
{
    // first find the pollutant in the map, if we didn't find it, read the file,
    // this should parse the whole set of stations & aggregation times, so no need
    // to re read the file afterwards, only return 0 when data is not found...
    auto it = _buffer.find(pollutant);
    if (it == _buffer.end())
    {
        // not found: read data file..
        readFile(pollutant);
        // and fetch it again
        it = _buffer.find(pollutant);
    }

    if (it == _buffer.end())
    {
        return nullptr;
    }

    // lookup the aggregation
    auto& buffer = it->second;
    auto it2 = buffer.find(aggr);
    if (it2 == buffer.end())
    {
        return nullptr; // we should have it already once the file is completely read in
    }

    // lookup the station
    auto& buffer2 = it2->second;
    auto it3 = buffer2.find(station);
    if (it3 == buffer2.end())
    {
        return nullptr;
    }

    return &(it3->second);
}

// parse the file and read in the pollutant //
// no need to specify the aggregation time since we load the whole file in memory and build the
// map in one go...
void RioObsProvider::readFile(const std::string& pollutant)
{
    // create file name & open file stream
    std::string filename = _pattern;
    StringTools::replaceAll(filename, POLLUTANT_PLACEHOLDER, pollutant);

    std::ifstream file(filename.c_str());
    if (!file.is_open())
    {
        _logger->warn("Failed to open file: {}", filename);
        return;
    }

    _buffer[pollutant] = readObservationsFile(file, _AQNetworkProvider->getAQNetwork(), _nvalues, _timeResolution);
}

}

OPAQ_REGISTER_PLUGIN(OPAQ::RioObsProvider);
