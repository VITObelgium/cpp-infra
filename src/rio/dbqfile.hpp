#pragma once

#include "obshandler.hpp"
#include "parser.hpp"
#include "timeseries.hpp"

namespace rio {

class dbqfile : public obshandler
{
public:
    dbqfile(TiXmlElement* cnf);
    dbqfile(std::string filename, std::string type, double scale = 1.);
    virtual ~dbqfile();

    // retrieve the data from the file
    // exclude missing stations
    void get(std::map<std::string, double>& data,
        boost::posix_time::ptime tstart, std::string pol, std::string agg);

private:
    std::map<std::string, rio::timeseries> _db1h;
    std::map<std::string, rio::timeseries> _dbda;
    std::map<std::string, rio::timeseries> _dbm1;
    std::map<std::string, rio::timeseries> _dbm8;

    double _scale; //! value to scale the input data with : internally the values will be stored as _scale * [what is read]

    void load_riofile(std::string filename);

    void write_summary(void);
};

}
