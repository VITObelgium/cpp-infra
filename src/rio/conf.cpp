#include "conf.hpp"
#include "infra/exception.h"
#include "parser.hpp"
#include "strfun.hpp"
#include "xmltools.hpp"

#include <boost/program_options.hpp>

#include <iostream>
#include <stdexcept>

namespace rio {

using namespace inf;
namespace po = boost::program_options;

config::config()
: _tmode(0)
{
    // create the supported formates for date/time parsing
    _formats.push_back(std::locale(std::locale::classic(), new boost::posix_time::time_input_facet("%Y-%m-%d %H:%M:%S")));
    _formats.push_back(std::locale(std::locale::classic(), new boost::posix_time::time_input_facet("%Y-%m-%d")));
}

boost::posix_time::ptime config::_parse_datetime(const char* s)
{
    boost::posix_time::ptime pt;
    for (size_t i = 0; i < _formats.size(); ++i) {
        std::istringstream is(s);
        is.imbue(_formats[i]);
        is >> pt;
        if (pt != boost::posix_time::ptime()) return pt;
    }

    return boost::posix_time::ptime();
}

void config::_get_defaults(XmlNode el)
{
    if (!el) {
        throw RuntimeError("no defaults in setup xml file...");
    }

    std::cout << "Reading defaults from setup file..." << std::endl;
    _base = el.child("base_path").optional_trimmed_value().value_or(".");

    if (_cnf.empty()) {
        _cnf = el.child("configuration").trimmed_value();
    }

    if (_pol.empty()) {
        _pol = el.child("pollutant").trimmed_value();
    }

    if (_aggr.empty()) {
        _aggr = el.child("aggregation").trimmed_value();
    }

    if (_ipol.empty()) {
        _ipol = el.child("mapper").trimmed_value();
    }

    if (_grd.empty()) {
        _grd = el.child("grid").trimmed_value();
    }

    if (_out.empty()) {
        _out = {std::string(el.child("output").trimmed_value())};
    }
}

// process the XML with the selected configuration elemetns
void config::_get_runconfig()
{
    if (!_domRoot) {
        throw RuntimeError("_configure() :: no <rio> root element in setup file");
    }

    // find the requested configuration
    auto cnfEl = rio::xml::getElementByAttribute(_domRoot, "configuration", "name", _cnf);
    if (!cnfEl) {
        throw RuntimeError("Cannot find requested configuration {} in XML setup file", _cnf);
    }

    // some general, configuration specific configuration
    _author        = cnfEl.child("author").value();
    _email         = cnfEl.child("email").value();
    _desc          = cnfEl.child("description").value();
    _missing_value = cnfEl.child("missing_value").value<double>().value_or(-9999.0);

    // find a *generic* station and grid tag, can be overwritten later on
    _stationConfig = cnfEl.child("stations");
    _gridConfig    = cnfEl.child("grids");

    // find the observations
    _obsConfig = cnfEl.child("observations");
    if (!_obsConfig) {
        throw RuntimeError("cannot find <observations> element in configuration {}", _cnf);
    }

    // get the model configuration
    XmlNode polEl;
    try {
        std::vector<std::string> attNames = {"name", "aggr"};
        std::vector<std::string> attVals  = {_pol, _aggr};
        polEl                             = rio::xml::getElementByAttributesList(cnfEl, "pollutant", attNames, attVals);

    } catch (...) {
        throw RuntimeError("cannot find <pollutant> element with name={} and aggr={} in configuration {}", _pol, _aggr, _cnf);
    }

    // some additional pollutant specific configuration
    _detection_limit = polEl.child("detection_limit").value<double>().value_or(1.0);

    // now check the model
    try {
        _modelConfig = rio::xml::getElementByAttribute(polEl, "mapper", "name", _ipol);
    } catch (...) {
        throw RuntimeError("cannot find <mapper> element with name={} for {}, {}", _ipol, _pol, _aggr);
    }

    // Overwrite generic station and grid with the one inside the model, check later
    if (auto p = _modelConfig.child("stations"); p) {
        std::cout << "Overwriting <stations> with " + _ipol + " model specific configuration" << std::endl;
        _stationConfig = p;
    }

    if (auto p = _modelConfig.child("grids"); p) {
        std::cout << "Overwriting <grids> with " << _ipol << " model specific configuration" << std::endl;
        _gridConfig = p;
    }

    // get some info from the model
    _ipol_class = _modelConfig.attribute("class");

    // do we have everything ?
    if (!_stationConfig) {
        throw RuntimeError("cannot find <stations> element in configuration {}", _cnf);
    }

    if (!_gridConfig) {
        throw RuntimeError("cannot find <grids> element in configuration {}", _cnf);
    }
}

void config::parse_setup_file(const std::string& setup_file)
{
    // try to import the setup file
    std::cout << "Using deployment in : " << setup_file << std::endl;
    _dom = XmlDocument::load_from_file(setup_file);
    if (!_dom) {
        throw RuntimeError("unable to load/parse xml setupfile: {}", setup_file);
    }

    _domRoot = _dom.root_node();
    if (!_domRoot || _domRoot.name() != "rio") {
        throw RuntimeError("no <rio> root element in setup file");
    }

    // set some defaults from DOM
    _get_defaults(_domRoot.child("defaults"));

    // get output configuration
    _outputConfig = _domRoot.child("output");
    if (!_outputConfig) {
        throw RuntimeError("cannot find <output> element in setup file");
    }

    // read the xml file and parse
    _get_runconfig();

    // update rio parser
    _update_parser();
}

void config::_update_parser()
{
    // set the default wildcards, the time will be set in the loop
    rio::parser::get()->add_pattern("%base%", _base);
    rio::parser::get()->add_pattern("%cnf%", _cnf);
    rio::parser::get()->add_pattern("%pol%", _pol);
    rio::parser::get()->add_pattern("%aggr%", _aggr);
    rio::parser::get()->add_pattern("%ipol%", _ipol);
    rio::parser::get()->add_pattern("%grid%", _grd);

    // add start and end times for the requested run
    rio::parser::get()->add_pattern("%start_time%", boost::posix_time::to_iso_string(_tstart));
    rio::parser::get()->add_pattern("%end_time%", boost::posix_time::to_iso_string(_tstop));

    return;
}

void config::parse_command_line(int argc, char* argv[])
{
    po::options_description optionsDesc("Available options");
    optionsDesc.add_options()("base,b", po::value<std::string>(&_base)->default_value(".")->value_name("<path>"), "base path for this run")("cnf,c", po::value<std::string>(&_cnf)->value_name("<name>"), "RIO configuration")("pol,p", po::value<std::string>(&_pol)->value_name("<name>"), "pollutant")("aggr,a", po::value<std::string>(&_aggr)->value_name("<1h,da,m1,m8,wk>"), "aggregation")("ipol,i", po::value<std::string>(&_ipol)->value_name("<name>"), "interpolation model")("grid,g", po::value<std::string>(&_grd)->value_name("<name>"), "grid name")("output,o", po::value<std::string>()->default_value("")->value_name("<type1>[,type2,...]"), "output option, comma separated [txt,hdf5,...]")("tmode,t", po::value<int>(&_tmode)->value_name("[0,1]"), "timestamp convention")("help,h", "show this message.")("debug,d", "switch on debugging mode");

    po::options_description pidesc("Position independant options");
    pidesc.add_options()("tstart", po::value<std::string>()->default_value(""), "start time")("tstop", po::value<std::string>()->default_value(""), "stop time");

    po::options_description all("All options");
    all.add(optionsDesc).add(pidesc);

    po::positional_options_description pd;
    pd.add("tstart", 1).add("tstop", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(all).positional(pd).run(), vm);
    po::notify(vm);

    if (vm.count("help") > 0) {
        //std::cout << desc << std::endl;
        std::cout << "Usage:\n"
                  << "  rio [options] tstart [tstop]\n\n"
                  << optionsDesc
                  << "Supported datetime format : yyyy-mm-dd[ HH:MM:SS]" << std::endl;
        std::exit(EXIT_SUCCESS);
    }

    if (vm.count("output") > 0) {
        // parse the list of options
        boost::split(_out, vm["output"].as<std::string>(), boost::is_any_of(",;"));

        // handle empty ones if the user should put a trailing comma or semi colon after the list
        auto it = std::remove_if(_out.begin(), _out.end(), [](const std::string& s) { return s.empty(); });
        _out.erase(it, _out.end());
    }

    _debug = vm.count("debug") > 0;

    // parsing date/time
    if (vm.count("tstart") == 1) {
        _tstart = _parse_datetime(vm["tstart"].as<std::string>().c_str());
    } else {
        throw RuntimeError("*** error parsing command line options, see --help !");
    }

    if (vm.count("tstop") == 1) {
        _tstop = _parse_datetime(vm["tstop"].as<std::string>().c_str());
        if (_tstop.is_not_a_date_time()) {
            // fix as the argument is there as "" with the boost argument parser.. grmmbl
            std::cout << "Requested single date : " << _tstart << std::endl;
            _tstop = _tstart;
        } else {
            std::cout << "Requested date range : " << _tstart << " to " << _tstop << std::endl;
        }
    } else {
        std::cout << "Requested single date : " << _tstart << std::endl;
        _tstop = _tstart;
    }

    // set timestep and adjust the requeste start time and end time to the beginning of the interval
    if (!_aggr.compare("1h")) {
        _tstep = boost::posix_time::hours(1);
    } else
        _tstep = boost::posix_time::hours(24);

    // check timemode convention
    if (_tstep.hours() < 24) {
        if ((_tmode != 0) && (_tmode != 1))
            throw RuntimeError("please use 0 or 1 for timestamp convention...");

        // if the user says the time mode is 1, then the given timestamps at the command line are after the hour,
        // convert to internal 'before the hour' convention... 
        if (_tmode == 1) {
            std::cout << "Timestamp convention given after hour, converting to internal convention...\n";
            _tstart -= _tstep;
            _tstop -= _tstep;
        }
    }   
}

std::ostream& operator<<(std::ostream& output, const config& c)
{
    output << "[Configuration]" << std::endl;
    output << " Debugging mode   : " << (c._debug ? "on" : "off") << std::endl;
    output << " Base path        : " << c._base << std::endl;
    output << " Configuration    : " << c._cnf << std::endl;
    output << " Pollutant        : " << c._pol << std::endl;
    output << " Aggregation      : " << c._aggr << std::endl;
    output << " Interpolator     : " << c._ipol << std::endl;
    output << " Grid             : " << c._grd << std::endl;

    output << " Output mode      :";
    for (auto s : c._out)
        output << " " << s;
    output << std::endl;

    output << "[Date range]" << std::endl;
    output << " Internal start time        : " << boost::posix_time::to_simple_string(c._tstart) << std::endl;
    output << " Internal stop time         : " << boost::posix_time::to_simple_string(c._tstop) << std::endl;
    if (c._tstep.hours() < 24)
        output << " Input timestamp convention : " << (c._tmode == 0 ? "before the hour" : "after the hour") << std::endl;
    return output; // for multiple << operators.
}

} //namespace
