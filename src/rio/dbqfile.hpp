#pragma once

#include "obshandler.hpp"
#include "parser.hpp"
#include "timeseries.hpp"

namespace inf {
class XmlNode;
}

namespace rio {

class dbqfile : public obshandler
{
public:
    dbqfile(const inf::XmlNode& cnf);
    dbqfile(std::string filename, std::string type, double scale = 1.);
    virtual ~dbqfile();

    // retrieve the data from the file
    // exclude missing stations
    std::unordered_map<std::string, double> get(boost::posix_time::ptime tstart, std::string pol, std::string agg) override;
    boost::posix_time::ptime first_time() const override;
    boost::posix_time::ptime last_time() const override;

private:
    std::unordered_map<std::string, rio::timeseries> _db1h;
    std::unordered_map<std::string, rio::timeseries> _dbda;
    std::unordered_map<std::string, rio::timeseries> _dbm1;
    std::unordered_map<std::string, rio::timeseries> _dbm8;

    double _scale; //! value to scale the input data with : internally the values will be stored as _scale * [what is read]

    void load_riofile(std::string filename);

    // implementation using mmap en zero copy approach (roughly 6x to 8x speedup :-)
    void load_riofile_faster(const std::string& filename);

    void write_summary();
};

}
