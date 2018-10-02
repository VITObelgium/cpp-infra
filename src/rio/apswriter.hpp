#pragma once

#include "outputhandler.hpp"

namespace rio {

class apswriter : public outputhandler
{
public:
    apswriter(const inf::XmlNode& cnf);
    ~apswriter();

    void init(const rio::config& cnf,
        const std::shared_ptr<rio::network> net,
        const std::shared_ptr<rio::grid> grid);

    void write(const boost::posix_time::ptime& curr_time,
        const std::map<std::string, double>& obs,
        const Eigen::VectorXd& values,
        const Eigen::VectorXd& uncert);

    void close(void);

public:
    typedef struct
    {
        std::string aggr;
        std::string pol;
        std::string units;
        std::string origin;
        std::string comments;
        std::string fmt;
        std::string c_fmt;

        int srs; //! spatial reference system code (1)
        double xull;
        double yull;
        double dx;
        double dy;
        double missing;

        int nx;
        int ny;

        std::string mapfile_pattern;
    } apsconf_t;

    typedef struct
    {
        int I;
        int J;
    } apsidx_t;

private:
    // some configuration settings
    std::string _pattern_values;   //! output filename pattern
    std::string _pattern_uncert;   //! uncertainties filename pattern
    apsconf_t _apscf;              //! aps header config
    std::vector<apsidx_t> _apsmap; //! vector, assuming same size as grid with aps I,J values

    std::shared_ptr<rio::network> _net;
    std::shared_ptr<rio::grid> _grid;

    std::vector<double> _buffer;

private:
    void write_header(FILE* fp, const boost::posix_time::ptime& curr_time, const std::string& comments);
    void write_buffer(FILE* fp);
};

}
