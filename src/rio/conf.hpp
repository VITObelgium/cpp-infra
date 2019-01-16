#pragma once

#include "griddefinition.hpp"
#include "infra/xmldocument.h"

#include <memory>
#include <string>
#include <vector>

#include <boost/date_time.hpp>

namespace rio {

class config
{
public:
    config();

    void parse_setup_file(const std::string& setup_file);
    void parse_command_line(int argc, char* argv[]);

    void apply_config(
        std::string_view config_name,
        std::string_view pollutant,
        std::string_view interpolation,
        std::string_view aggregation,
        std::string_view grid,
        std::string_view startDate,
        std::string_view endDate);

    bool debug() const
    {
        return _debug;
    }

    friend std::ostream& operator<<(std::ostream& output, const config& c);

    std::string base_path() const
    {
        return _base;
    }

    std::string pol() const
    {
        return _pol;
    }

    // returns the available pollutants for the provided configuration
    std::vector<std::string_view> pol_names(std::string_view config) const;

    std::string aggr() const
    {
        return _aggr;
    }

    // returns the available aggregation periods for the pollutant in the specified configuration
    std::vector<std::string_view> aggr_names(std::string_view config, std::string_view pollutant) const;

    std::string grid() const
    {
        return _grd;
    }

    // returns the available grids for the provided configuration
    std::vector<std::string_view> grid_names(std::string_view config) const;

    // returns the available interpolation methods for the pollutant in the specified configuration
    std::vector<std::string_view> ipol_names(std::string_view config, std::string_view pollutant) const;

    std::string ipol() const
    {
        return _ipol;
    }

    std::string configuration() const
    {
        return _cnf;
    }

    boost::posix_time::ptime start_time() const
    {
        return _tstart;
    }

    boost::posix_time::ptime stop_time() const
    {
        return _tstop;
    }

    boost::posix_time::time_duration tstep() const
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

    std::unordered_map<std::string, griddefinition> grid_definitions() const;

    std::string_view desc(std::string_view config) const;
    std::vector<std::string_view> configurations() const;

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
    inf::XmlNode _outputConfig; //! and for the output
    inf::XmlNode _pollutant;    //! pollutant config
    inf::XmlNode _modelConfig;  //! model config

    std::string _ipol_class;
    double _detection_limit;
    double _missing_value;
    std::string _author;
    std::string _email;

private:
    // some helper routines
    void _get_defaults(inf::XmlNode el);
    void _get_runconfig();
    void _configure_timestep();
    void _update_parser();
    inf::XmlNode _get_config_node(std::string_view config) const;
    inf::XmlNode _get_pollutant_node(inf::XmlNode configNode, std::string_view pollutantName, std::string aggregation = "") const;

    std::vector<std::locale> _formats;
    boost::posix_time::ptime _parse_datetime(std::string_view s);
};
}
