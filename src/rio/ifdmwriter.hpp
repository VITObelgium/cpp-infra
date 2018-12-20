#pragma once

#include <cstdio>
#include <memory>
#include <string>

#include "outputhandler.hpp"

namespace rio {

class ifdmwriter : public outputhandler
{
public:
    ifdmwriter(const inf::XmlNode& cnf);
    ~ifdmwriter();

    void init(const rio::config& cnf,
        const std::shared_ptr<rio::network> net,
        const std::shared_ptr<rio::grid> grid);

    void write(const boost::posix_time::ptime& curr_time,
        const std::map<std::string, double>& obs,
        const Eigen::VectorXd& values,
        const Eigen::VectorXd& uncert);

    void close(void);

    typedef struct
    {
        int row;
        int col;
    } grididx_t;

private:
    std::string _pattern; //! output filename pattern

    std::shared_ptr<rio::network> _net;
    std::shared_ptr<rio::grid> _grid;

    FILE* _fs;

    float _xul; // world x coordinate of upper left corner (grid edge point, not center point)
    float _yul; // world y coordinate of upper left corner
    float _dx;  // x resolution fo grid
    float _dy;  // y resolution of grid grid size
    int _epsg;  // grid projection code

    int _missing; // missing value

    int _nx; // number of columns in raster output
    int _ny; // number of rows in raster output

    int _nt; // number of timesteps in file

    // timestamp mode
    int _tmode; // 0: t0 is timestamp at start of measurement interval
                // 1: t0 is timestamp at end of interval (default, probably)

    std::string _mapfile_pattern;

    std::vector<grididx_t> _griddef;

    std::vector<float> _buffer; // 32 bit floating point buffer

    // helper routine
    void fwrite_string(const std::string& s, unsigned int n);
};
}
