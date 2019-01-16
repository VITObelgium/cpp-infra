#include "dbqfile.hpp"
#include "infra/exception.h"
#include "infra/log.h"
#include "infra/string.h"
#include "infra/xmldocument.h"
#include "strfun.hpp"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/date_time.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <fmt/ostream.h>
#include <stdexcept>

using namespace boost::gregorian;
using namespace boost::posix_time;

namespace rio {

using namespace inf;

dbqfile::dbqfile(const XmlNode& cnf)
{
    /*
     <observations>
      <dbqfile type="RIO" scale="1.0">%base%/data/%pol%_data_rio.txt</dbqfile>
    </observations>
  */
    if (!cnf) {
        throw std::runtime_error("Invalid TiXmlElement pointer in dbqfile()...");
    }

    auto fEl = cnf.child("dbqfile");
    if (!fEl) {
        throw std::runtime_error("Cannot find <dbqfile> element in observations...");
    }

    // get the scale attribute
    _scale = str::to_numeric<double>(fEl.attribute("scale")).value_or(1.0);

    // get the type attribute
    auto typeAttr = fEl.optional_attribute("type");
    if (!typeAttr.has_value()) {
        std::cout << "+++ warning: assuming dbqfile type RIO, no type attribute given in <dbqfile>..." << std::endl;
    }

    std::string type(typeAttr.value_or("RIO"));

    // get the filename
    std::string filename(fEl.value());

    // run through the parser
    rio::parser::get()->process(filename);

    // load the file
    if (boost::iequals(type, "RIO")) {
        load_riofile(filename);
    } else {
        throw RuntimeError("File type {} is not supported by dbqfile", type);
    }
}

dbqfile::dbqfile(std::string filename, std::string type, double scale)
: _scale(scale)
{
    if (boost::iequals(type, "RIO")) {
        load_riofile_faster(filename);
    } else {
        throw std::runtime_error("File type " + type + " is not supported by dbqfile");
    }
}

void dbqfile::load_riofile(std::string filename)
{
    Log::debug("Importing {}", filename);
    // parse whole file & store in database...

    FILE* fp = fopen(filename.c_str(), "r");
    if (!fp) throw std::runtime_error("Unable to open file: " + filename);

    char line[rio::strfun::LINESIZE];
    char* ptok;

    while (fgets(line, sizeof(line), fp)) {
        if (rio::strfun::trim(line)[0] == '#') continue;
        if (!strlen(rio::strfun::trim(line))) continue;

        if (!(ptok = strtok(line, rio::strfun::SEPCHAR))) continue;
        std::string st_name = ptok;
        if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
        std::string date_str = ptok;
        int year             = atoi(date_str.substr(0, 4).c_str());
        int mon              = atoi(date_str.substr(4, 2).c_str());
        int day              = atoi(date_str.substr(6, 2).c_str());

        // construct the date& time periods
        boost::posix_time::ptime t1(boost::gregorian::date(year, mon, day));
        boost::posix_time::time_period dt(t1, hours(24));

        // m1
        if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
        double m1 = atof(ptok);
        _dbm1[st_name].insert(dt, _scale * m1);

        // m8
        if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
        double m8 = atof(ptok);
        _dbm8[st_name].insert(dt, _scale * m8);

        // da
        if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
        double da = atof(ptok);
        _dbda[st_name].insert(dt, _scale * da);

        // 24 hourly values
        boost::posix_time::time_period dt1h(t1, hours(1));
        while (ptok = strtok(NULL, rio::strfun::SEPCHAR)) {
            double xx1h = atof(ptok);
            _db1h[st_name].insert(dt1h, _scale * xx1h);
            dt1h.shift(hours(1));
        }
    }

    fclose(fp);

    // write some info about the timeseries...
    //write_summary();
    return;
}

void dbqfile::load_riofile_faster(const std::string& filename)
{
    Log::debug("Importing {}", filename);
    // parse whole file & store in database...

    boost::iostreams::mapped_file_source mmapFile(filename);
    if (!mmapFile.is_open()) {
        throw RuntimeError("Failed to open {}", filename);
    }

    std::string_view fileContents(mmapFile.data(), mmapFile.size());
    auto lines = str::split_view(fileContents, '\n', str::SplitOpt::Trim);

    for (auto& line : lines) {
        if (line.empty() || line[0] == '#') continue;

        auto items = str::split_view(line, " \t;,", str::StrTokFlags);

        if (items.size() != 29) {
            Log::warn("Invalid dbq line: {}", line);
            continue;
        }

        auto name = std::string(items[0]);
        auto date = items[1];

        if (date.size() != 8) {
            Log::warn("Invalid date length in dbq line: {}", line);
            continue;
        }

        auto year = str::to_int32(date.substr(0, 4));
        auto mon  = str::to_int32(date.substr(4, 2));
        auto day  = str::to_int32(date.substr(6, 2));

        if (!(year.has_value() && mon.has_value() && day.has_value())) {
            Log::warn("Invalid date in dbq line: {}", line);
            continue;
        }

        // construct the date& time periods
        boost::posix_time::ptime t1(boost::gregorian::date(*year, *mon, *day));
        boost::posix_time::time_period dt(t1, hours(24));

        // m1
        auto m1 = str::to_double(items[2]);
        if (!m1.has_value()) {
            continue;
        }

        _dbm1[name].insert(dt, _scale * *m1);

        // m8
        auto m8 = str::to_double(items[3]);
        if (!m8.has_value()) {
            continue;
        }
        _dbm8[name].insert(dt, _scale * *m8);

        // da
        auto da = str::to_double(items[4]);
        if (!da.has_value()) {
            continue;
        }
        _dbda[name].insert(dt, _scale * *da);

        // 24 hourly values
        boost::posix_time::time_period dt1h(t1, hours(1));
        for (int i = 5; i < 5 + 24; ++i) {
            _db1h[name].insert(dt1h, _scale * str::to_double(items[i]).value_or(-9999.0));
            dt1h.shift(hours(1));
        }
    }
}

dbqfile::~dbqfile()
{
}

std::unordered_map<std::string, double> dbqfile::get(boost::posix_time::ptime tstart, std::string pol, std::string agg)
{
    std::unordered_map<std::string, double> data;

    // get pointer to correct database archive, depending on aggregation
    std::unordered_map<std::string, rio::timeseries>* _db;
    if (!agg.compare("m1")) {
        _db = &_dbm1;
    } else if (!agg.compare("m8")) {
        _db = &_dbm8;
    } else if (!agg.compare("da")) {
        _db = &_dbda;
    } else if (!agg.compare("1h")) {
        _db = &_db1h;
    } else {
        throw std::runtime_error("no such aggregation time : " + agg);
    }

    bool missing = false;
    for (const auto& it : *_db) {
        double x = it.second.get(tstart, missing);
        if (!missing) {
            // if we have set the network, only return data for stations in the network, otherwise return all
            if (_net) {
                if (!_net->get(it.first)) continue;
            }

            data.emplace(it.first, x); // otherwise don't insert in the map
        }
    }

    return data;
}

boost::posix_time::ptime dbqfile::first_time() const
{
    boost::posix_time::ptime first;
    for (const auto& [name, ts] : _dbda) {
        (void)name;
        first = std::min(ts.first_time(), first);
    }

    for (const auto& [name, ts] : _dbm1) {
        (void)name;
        first = std::min(ts.first_time(), first);
    }

    for (const auto& [name, ts] : _dbm8) {
        (void)name;
        first = std::min(ts.first_time(), first);
    }

    for (const auto& [name, ts] : _db1h) {
        (void)name;
        first = std::min(ts.first_time(), first);
    }

    return first;
}

boost::posix_time::ptime dbqfile::last_time() const
{
    boost::posix_time::ptime last = from_time_t(0);
    for (const auto& [name, ts] : _dbda) {
        (void)name;
        last = std::max(ts.last_time(), last);
    }

    for (const auto& [name, ts] : _dbm1) {
        (void)name;
        last = std::max(ts.last_time(), last);
    }

    for (const auto& [name, ts] : _dbm8) {
        (void)name;
        last = std::max(ts.last_time(), last);
    }

    for (const auto& [name, ts] : _db1h) {
        (void)name;
        last = std::max(ts.last_time(), last);
    }

    return last;
}

void dbqfile::write_summary()
{
    Log::debug("[List of stations]");
    for (const auto& it : _dbm1) {
        Log::debug(it.first);
        Log::debug(" - da range : {} - {}, step: {}, size: {}", _dbda[it.first].first_time(), _dbda[it.first].last_time(), _dbda[it.first].interval(), _dbda[it.first].size());
        Log::debug(" - m1 range : {} - {}, step: {}, size: {}", _dbm1[it.first].first_time(), _dbm1[it.first].last_time(), _dbm1[it.first].interval(), _dbm1[it.first].size());
        Log::debug(" - m8 range : {} - {}, step: {}, size: {}", _dbm8[it.first].first_time(), _dbm8[it.first].last_time(), _dbm8[it.first].interval(), _dbm8[it.first].size());
        Log::debug(" - 1h range : {} - {}, step: {}, size: {}", _db1h[it.first].first_time(), _db1h[it.first].last_time(), _db1h[it.first].interval(), _db1h[it.first].size());
    }

    return;
}
}
