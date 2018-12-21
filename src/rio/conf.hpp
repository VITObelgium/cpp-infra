#pragma once

#include "infra/xmldocument.h"

#include <memory>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>

namespace rio {

class config
{
public:
    config();

    void parse_setup_file(const std::string& setup_file);
    void parse_command_line(int argc, char* argv[]);

    bool debug(void) const
    {
        return _debug;
    }

    friend std::ostream& operator<<(std::ostream& output, const config& c);

    std::string base_path(void) const
    {
        return _base;
    }

    std::string pol(void) const
    {
        return _pol;
    }

    std::string aggr(void) const
    {
        return _aggr;
    }

    std::string grid(void) const
    {
        return _grd;
    }

    std::string ipol(void) const
    {
        return _ipol;
    }

    std::string configuration(void) const
    {
        return _cnf;
    }

    boost::posix_time::ptime start_time(void) const
    {
        return _tstart;
    }

    boost::posix_time::ptime stop_time(void) const
    {
        return _tstop;
    }

    boost::posix_time::time_duration tstep(void) const
    {
        return _tstep;
    }

    inf::XmlNode stationConfig() const
    {
        return _stationConfig;
    }

    inf::XmlNode gridConfig() const
    {
        return _gridConfig;
    }

    inf::XmlNode obsConfig() const
    {
        return _obsConfig;
    }

    inf::XmlNode modelConfig() const
    {
        return _modelConfig;
    }

    inf::XmlNode outputConfig() const
    {
        return _outputConfig;
    }

    const std::vector<std::string>& req_output() const
    {
        return _out;
    }

    // mapping model selection
    const std::string& ipol_class() const
    {
        return _ipol_class;
    }

    double detection_limit() const
    {
        return _detection_limit;
    }

    double missing_value() const
    {
        return _missing_value;
    }

    const std::string& author() const
    {
        return _author;
    }

    const std::string& email() const
    {
        return _email;
    }

    const std::string& desc() const
    {
        return _desc;
    }

private:
    bool _debug;

    std::string _base; //! basepath
    std::string _cnf;  //! configuration

    std::string _pol;  //! running for pollutant
    std::string _aggr; //! running for this aggregation time
    std::string _grd;  //! running for this grid
    std::string _ipol; //! mapper

    std::vector<std::string> _out; //! output options

    int _tmode; //! timestamp convention for command line options only for sub daily aggregations
                //! 0: the requested timestamps at the command line indicate times before the hour
                //! 1: the requested timestamps is after the hour
    boost::posix_time::ptime _tstart;
    boost::posix_time::ptime _tstop;
    boost::posix_time::time_duration _tstep;

    inf::XmlDocument _dom; //! setup xml document model
    inf::XmlNode _domRoot; //! root element in DOM

    inf::XmlNode _stationConfig; //! pointer to the station config element

    // replace this...
    inf::XmlNode _gridConfig; //! ditto for the grid

    inf::XmlNode _obsConfig;    //! ditto for the observations
    inf::XmlNode _outputConfig; //! and for the outpu
    inf::XmlNode _modelConfig;  //! model config

    std::string _ipol_class;
    double _detection_limit;
    double _missing_value;
    std::string _author;
    std::string _email;
    std::string _desc;

private:
    // some helper routines
    void _get_defaults(inf::XmlNode el);
    void _get_runconfig(void);
    void _update_parser(void);

    std::vector<std::locale> _formats;
    boost::posix_time::ptime _parse_datetime(const char* s);
};
}
