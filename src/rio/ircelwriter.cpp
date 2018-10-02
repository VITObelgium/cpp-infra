#include <fstream>

#include "ircelwriter.hpp"
#include "parser.hpp"

namespace rio {

ircelwriter::ircelwriter(const inf::XmlNode& cnf)
: outputhandler(cnf)
, _saroad_codes{{"o3", "44201"}, {"no2", "42602"}, {"pm10", "88102"},
      {"pm25", "81104"}, {"so2", "42401"}, {"co", "42101"},
      {"bzn", "45201"}, {"bc", "16111"}, {"no", "42601"}}
, _dateFmt{std::locale::classic(), new boost::gregorian::date_facet("%Y%d%m")}
, _pattern("rio_%timestamp%.txt")
, _curr_day(boost::gregorian::min_date_time)
{
    try {
        _pattern = _xml.get<std::string>("handler.location");
    } catch (...) {
        throw std::runtime_error("invalid configuration in ircelwriter XML config, <location> required");
    }
}

ircelwriter::~ircelwriter()
{
}

void ircelwriter::write_buffers(const std::string& fname)
{
    FILE* fp = fopen(fname.c_str(), "w");

    std::ostringstream date_s;
    date_s.imbue(_dateFmt);
    date_s << _curr_day;

    for (unsigned int i = 0; i < _valbuf.rows(); i++) {
        fprintf(fp, "%5d;%s;%s", i + 1, date_s.str().c_str(), _saroad.c_str());
        for (unsigned int j = 0; j < _valbuf.cols(); j++)
            fprintf(fp, ";%.2f", _valbuf(i, j));
        fprintf(fp, "\n");
    }

    fclose(fp);

    return;
}

void ircelwriter::init(const rio::config& cnf,
    const std::shared_ptr<rio::network> net,
    const std::shared_ptr<rio::grid> grid)
{
    _net  = net;
    _grid = grid;

    if (!cnf.aggr().compare("1h")) {
        _valbuf.resize(grid->size(), 24);
        _uncbuf.resize(grid->size(), 24);

        //TODO : add missing value from xml input config
        _valbuf.fill(-9999.);
        _uncbuf.fill(-9999.);

    } else {
        throw std::runtime_error("ircel writer only implemented for 1h aggregation");
    }

    auto it = _saroad_codes.find(cnf.pol());
    if (it == _saroad_codes.end()) throw std::runtime_error("saroad code not available for " + cnf.pol());
    _saroad = _saroad_codes.find(cnf.pol())->second;

    return;
}

void ircelwriter::write(const boost::posix_time::ptime& curr_time,
    const std::map<std::string, double>& /*obs*/,
    const Eigen::VectorXd& values,
    const Eigen::VectorXd& uncert)
{
    boost::gregorian::date req_day = curr_time.date();
    if (req_day < _curr_day) {
        throw std::runtime_error("requested write for earlier date in ircelwriter...");
    } else if (req_day > _curr_day) {
        // change in date, start new file...
        _fname = _pattern;

        // carefull : this handles it's own internal date (updated in main loop)
        rio::parser::get()->process(_fname);

        _curr_day = req_day;
    }
    // get the hours
    unsigned int hr = curr_time.time_of_day().hours();

    // append the date to the buffers
    _valbuf.col(hr) = values;
    _uncbuf.col(hr) = uncert;

    // write after last hour of the day
    if (hr == 23) {
        write_buffers(_fname);

        // and reset buffers
        _valbuf.fill(-9999.);
        _uncbuf.fill(-9999.);
    }

    return;
}

void ircelwriter::close(void)
{
    std::cout << "Closing ircelwriter..." << std::endl;
    // final write...
    write_buffers(_fname);

    return;
}

}
