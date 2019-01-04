#include <algorithm>

#include "apswriter.hpp"
#include "parser.hpp"
#include "strfun.hpp"

namespace rio {

using namespace inf;

apswriter::apswriter(const XmlNode& cnf)
: outputhandler(cnf)
, _pattern_values("rio_%timestamp%.aps")
, _pattern_uncert("rio_%timestamp%.err.aps")
{
}

apswriter::~apswriter()
{
}

void apswriter::init(const rio::config& cnf,
    const std::shared_ptr<rio::network> net,
    const std::shared_ptr<rio::grid> grid)
{
    std::cout << "  Initializing aps writer..." << std::endl;
    _net  = net;
    _grid = grid;

    try {
        // set the output location
        _pattern_values = _xml.get<std::string>("handler.location.values");
        _pattern_uncert = _xml.get<std::string>("handler.location.uncertainty");
    } catch (...) {
        throw std::runtime_error("invalid configuration in apswriter XML config, no <values> or <uncertainty> under <location> tag");
    }

    try {
        // lookup the
        bool found = false;
        for (const auto& kv : _xml.get_child("handler")) {
            //std::cout << "checking aps element : " << kv.first << std::endl;
            if (!kv.first.compare("map")) {
                if (!kv.second.get<std::string>("<xmlattr>.grid").compare(cnf.grid())) {
                    std::cout << "  Found aps map for grid " << cnf.grid() << std::endl;

                    //grid="4x4" srs="1" fmt="F7.1" missing="-999.0"
                    //nx="70" ny="80" dx="4.0" dy="4.0" xull="0." yull="620."

                    _apscf.aggr     = cnf.aggr();
                    _apscf.pol      = cnf.pol();
                    _apscf.units    = "ug/m3";
                    _apscf.origin   = cnf.ipol(); // use this to store the interpolation mode
                    _apscf.comments = "";         // use this to store which gird : "concentration" or "uncertainty"
                    _apscf.fmt      = kv.second.get<std::string>("<xmlattr>.fmt");
                    if (!_apscf.fmt.compare("F7.1"))
                        _apscf.c_fmt = "%7.1f";
                    else
                        throw std::runtime_error("TODO: currently only F7.1 supported, need to implement flexible format");

                    _apscf.srs     = 1;
                    _apscf.xull    = kv.second.get<double>("<xmlattr>.xull");
                    _apscf.yull    = kv.second.get<double>("<xmlattr>.yull");
                    _apscf.dx      = kv.second.get<double>("<xmlattr>.dx");
                    _apscf.dy      = kv.second.get<double>("<xmlattr>.dy");
                    _apscf.missing = kv.second.get<double>("<xmlattr>.missing");

                    _apscf.nx = kv.second.get<int>("<xmlattr>.nx");
                    _apscf.ny = kv.second.get<int>("<xmlattr>.ny");

                    // std::cout << "nx=" << _apscf.nx << ", ny=" << _apscf.ny << std::endl;
                    // contents of the element is empty string...
                    // std::cout << "pattern : " << kv.second.get<std::string>("") << std::endl;
                    _apscf.mapfile_pattern = kv.second.get<std::string>("");
                    found                  = true;
                }
            }
        }
        if (!found) throw std::runtime_error("no matching <map> element for selected grid found... ");

    } catch (...) {
        throw std::runtime_error("error in apswriter, no appropriate <map> for selected grid required");
    }

    // now import the mapping file and setup the mapping arrays
    std::string mapfile = _apscf.mapfile_pattern;
    rio::parser::get()->process(mapfile);
    _apsmap.clear();
    try {
        FILE* fp = fopen(mapfile.c_str(), "r");
        if (!fp) throw std::runtime_error("Unable to open file: " + mapfile);

        char line[rio::strfun::LINESIZE];
        char* ptok;
        // read header
        fgets(line, sizeof(line), fp);
        // read file
        while (fgets(line, sizeof(line), fp)) {
            apsidx_t idx;

            if (rio::strfun::trim(line)[0] == '#') continue;
            if (!strlen(rio::strfun::trim(line))) continue;

            if (!(ptok = strtok(line, rio::strfun::SEPCHAR))) continue;
            if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
            idx.I = atoi(ptok);
            if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
            idx.J = atoi(ptok);

            _apsmap.push_back(idx);
        }

        fclose(fp);
    } catch (...) {
        throw std::runtime_error("error importing aps mapping file : " + mapfile);
    }

    if (_apsmap.size() != _grid->size()) throw std::runtime_error("aps mapping file does not match grid size, got:" +
                                                                  std::to_string(_apsmap.size()) + ", expected : " + std::to_string(_grid->size()));

    return;
}

void apswriter::write(const boost::posix_time::ptime& curr_time,
    const std::unordered_map<std::string, double>& /*obs*/,
    const Eigen::VectorXd& values,
    const Eigen::VectorXd& uncert)
{
    FILE* fp;

    // 1. writing the concentration file
    std::string ofname = _pattern_values;
    rio::parser::get()->process(ofname);
    fp = fopen(ofname.c_str(), "wt");
    if (!fp) {
        throw std::runtime_error("cannot open " + ofname);
    }
    write_header(fp, curr_time, "concentration");

    // prepare buffer
    _buffer.resize(_apscf.nx * _apscf.ny);
    std::fill(_buffer.begin(), _buffer.end(), _apscf.missing);
    for (unsigned int i = 0; i < values.size(); i++) {
        unsigned int ix = (_apsmap[i].I - 1) * _apscf.nx + (_apsmap[i].J - 1);
        if (ix >= _buffer.size()) throw std::runtime_error("aps map file indexes outside of bounds for gridcell " + std::to_string(i + 1) + " ...");
        _buffer[ix] = values(i);
    }
    write_buffer(fp);
    fclose(fp);

    // 2. writing the uncertainty file
    ofname = _pattern_uncert;
    rio::parser::get()->process(ofname);
    fp = fopen(ofname.c_str(), "wt");
    if (!fp) {
        throw std::runtime_error("cannot open " + ofname);
    }
    write_header(fp, curr_time, "uncertainty");

    // prepare buffer
    std::fill(_buffer.begin(), _buffer.end(), _apscf.missing);
    for (unsigned int i = 0; i < uncert.size(); i++) {
        unsigned int ix = (_apsmap[i].I - 1) * _apscf.nx + (_apsmap[i].J - 1);
        if (ix >= _buffer.size()) throw std::runtime_error("aps map file indexes outside of bounds for gridcell " + std::to_string(i + 1) + " ...");
        _buffer[ix] = uncert(i);
    }
    write_buffer(fp);
    fclose(fp);

    return;
}

void apswriter::write_buffer(FILE* fp)
{
    for (int i = 0; i < _apscf.ny; i++) {
        for (int j = 0; j < _apscf.nx; j++)
            fprintf(fp, _apscf.c_fmt.c_str(), _buffer[i * _apscf.nx + j]);
        fprintf(fp, "\n");
    }

    return;
}

void apswriter::write_header(FILE* fp, const boost::posix_time::ptime& curr_time, const std::string& comments)
{
    std::tm d = boost::posix_time::to_tm(curr_time);
    // write date & time
    fprintf(fp, "%3d%3d%3d%3lld ", (d.tm_year > 100 ? d.tm_year - 100 : d.tm_year), d.tm_mon + 1, d.tm_mday, curr_time.time_of_day().hours());

    // write pol, units etc..
    // note ! left justify strings as in fortran !!!!
    fprintf(fp, "%*s ", 10, _apscf.pol.c_str()); // except for pollutant : use right justification... funky thing from FORTRAN...
    fprintf(fp, "%-*s ", 10, _apscf.units.c_str());
    fprintf(fp, "%-*s ", 10, _apscf.origin.c_str());
    fprintf(fp, "%-*s ", 22, comments.c_str());
    fprintf(fp, "%-*s ", 6, _apscf.fmt.c_str());

    // and the rest...
    fprintf(fp, "%2d %8.3f %8.3f%3d%3d %8.3f %8.3f\n", _apscf.srs, _apscf.xull, _apscf.yull,
        _apscf.nx, _apscf.ny, _apscf.dx, _apscf.dy);
    return;
}

void apswriter::close(void)
{
    std::cout << "Closing apswriter..." << std::endl;
    return;
}

}
