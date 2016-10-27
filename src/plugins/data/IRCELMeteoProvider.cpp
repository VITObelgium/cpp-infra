/*
 * IRCELMeteoProvider.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "IRCELMeteoProvider.h"
#include "PluginRegistration.h"
#include "tools/GzipReader.h"
#include "tools/XmlTools.h"
#include "tools/StringTools.h"

#include <tinyxml.h>
#include <boost/lexical_cast.hpp>

namespace OPAQ
{

static const std::string s_meteo_placeholder     = "%meteo%";
static const std::string s_parameter_placeholder = "%param%";
static const std::string s_basetime_placeholder  = "%basetime%";

using namespace std::chrono_literals;

IRCELMeteoProvider::IRCELMeteoProvider()
: _logger("IRCELMeteoProvider")
, _configured(false)
, _nsteps(0)
, _bufferStartReq(false)
, _backsearch(3)
{
}

std::string IRCELMeteoProvider::name()
{
    return "ircelmeteoprovider";
}

// OPAQ::Component methods
void IRCELMeteoProvider::configure(TiXmlElement* configuration, const std::string& componentName, IEngine&)
{
    setName(componentName);

    // clear buffers
    _nodata.clear();
    _buffer.clear();

    // parse parameter configuration
    auto* parametersElement = configuration->FirstChildElement("parameters");
    if (!parametersElement)
    {
        throw BadConfigurationException("parameters element not found");
    }

    auto* pEl = parametersElement->FirstChildElement("parameter");
    while (pEl)
    {
        std::string id;
        double nodata;

        if (pEl->QueryStringAttribute("id", &id) != TIXML_SUCCESS || pEl->QueryDoubleAttribute("nodata", &nodata) != TIXML_SUCCESS)
        {
            throw BadConfigurationException("parameter must have id and nodata attributes defined");
        }

        _nodata.emplace(id, nodata);
        pEl = pEl->NextSiblingElement("parameter");
    }

    _pattern = XmlTools::getChildValue(configuration, "file_pattern");
    _backsearch = XmlTools::getChildValue<int>(configuration, "backward_search", _backsearch);
    _timeResolution = std::chrono::hours(XmlTools::getChildValue<int>(configuration, "resolution"));

    if ((_timeResolution != 1h) && (_timeResolution != 2h) && (_timeResolution != 3h) &&
        (_timeResolution != 4h) && (_timeResolution != 6h) && (_timeResolution != 8h))
    {
        throw BadConfigurationException("invalid resolution, valid values are 1, 2, 3, 4, 6 and 8 hours !");
    }

    _nsteps = 24h / _timeResolution;

    // -- parse start time
    TiXmlElement* dateEl = configuration->FirstChildElement("buffer_start");
    if (dateEl)
    {
        _bufferStartDate = chrono::from_date_string(dateEl->GetText());
        _bufferStartReq  = true;
    }

    _configured = true;
}

std::chrono::hours IRCELMeteoProvider::getTimeResolution()
{
    return _timeResolution;
}

double IRCELMeteoProvider::getNoData(const std::string& parameterId)
{
    throwIfNotConfigured();
    auto it = _nodata.find(parameterId);
    if (it == _nodata.end())
    {
        throw OPAQ::NotAvailableException("Meteo parameter {} is not configured!", parameterId);
    }

    return it->second;
}

OPAQ::TimeSeries<double> IRCELMeteoProvider::getValues(const chrono::date_time& t1,
                                                       const chrono::date_time& t2,
                                                       const std::string& meteoId,
                                                       const std::string& paramId)
{
    throwIfNotConfigured();

    if (t2 < t1)
    {
        throw RunTimeException("End date has to be after start date");
    }

    if (_baseTime == chrono::date_time())
    {
        throw RunTimeException("No basetime set");
    }

    if (_buffer.find(meteoId) == _buffer.end())
    {
        readFile(meteoId, paramId);
    }

    return _buffer[meteoId][paramId].select(t1, t2);
}

void IRCELMeteoProvider::throwIfNotConfigured()
{
    if (!_configured)
    {
        throw RunTimeException("IRCELMeteoProvider Not fully configured");
    }
}

void IRCELMeteoProvider::readFile(const std::string& meteoId, const std::string& parameterId)
{
    GzipReader reader;
    std::string filename;

    // -- Create file name for the current basetime, if it doesn't exist, check the previous
    //    basetime, up till the amount of days configured...
    auto checkDate = _baseTime;
    bool have_file = false;
    while ((checkDate >= (_baseTime - chrono::days(_backsearch))) && (!have_file))
    {
        filename = _pattern;
        StringTools::replaceAll(filename, s_meteo_placeholder, meteoId);
        StringTools::replaceAll(filename, s_parameter_placeholder, parameterId);
        StringTools::replaceAll(filename, s_basetime_placeholder, chrono::to_date_string(checkDate));

        // -- Read & parse file
        try
        {
            reader.open(filename);
            have_file = true;
        }
        catch (const IOException&)
        {
            checkDate -= chrono::days(1);
            _logger->warn("{} not found, checking previous day : {}", filename, chrono::to_date_string(checkDate));
        }
    }

    if (!have_file)
    {
        _logger->error("giving up : no meteo file found for {}, {}", meteoId, parameterId);
        return;
    }

    auto& ts = _buffer[meteoId][parameterId];

    std::string line = reader.readLine();
    while (!reader.eof())
    {
        /*
         * line format:
         * YYYYMMDD hour0 hour6 hour12 hour18     // older ECMWF data : every 6 hours
         * YYYYMMDD hour0 hour3 hour6 hour9 ...   // new ECMWF data : every 3 hours
         */

        StringTools::StringSplitter meteoSplitter(line, " \t\r\n\f");

        auto iter = meteoSplitter.begin();

        // parse the first token : the date
        auto begin = chrono::make_date_time(boost::lexical_cast<int>(iter->substr(0, 4)),
                                            boost::lexical_cast<int>(iter->substr(4, 2)),
                                            boost::lexical_cast<int>(iter->substr(6, 2)));

        ++iter;

        // check if we are beyond the buffer start (if requested...)
        if (_bufferStartReq && (begin < _bufferStartDate))
        {
            line = reader.readLine();
            continue;
        }

        // read in the line & insert into the database
        size_t parsedValues = 0;
        for (; iter != meteoSplitter.end(); ++iter)
        {
            ts.insert(begin, boost::lexical_cast<double>(*iter));
            begin += 24h / _nsteps;
            ++parsedValues;
        }

        if (parsedValues != _nsteps)
        {
            throw RunTimeException("Meteo format does not match the configuration");
        }

        line = reader.readLine();
    }
}

OPAQ_REGISTER_STATIC_PLUGIN(IRCELMeteoProvider)

}
