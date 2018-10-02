#pragma once

#include <memory>

#include <H5Cpp.h>

#include "outputhandler.hpp"

namespace rio {

class h5writer : public outputhandler
{
public:
    h5writer(const inf::XmlNode& cnf);
    ~h5writer();

    void init(const rio::config& cnf,
        const std::shared_ptr<rio::network> net,
        const std::shared_ptr<rio::grid> grid);

    void write(const boost::posix_time::ptime& curr_time,
        const std::map<std::string, double>& obs,
        const Eigen::VectorXd& values,
        const Eigen::VectorXd& uncert);

    void close(void);

private:
    std::shared_ptr<rio::network> _net;
    std::shared_ptr<rio::grid> _grid;

    std::string _h5FileName;
    std::unique_ptr<H5::H5File> _h5File;

    // the different groups in the RIO file
    H5::Group _stGrp; //! the stations group
    H5::Group _grGrp; //! the grid group
    H5::Group _tmGrp; //! the time group

    H5::DataSet _stDS;    //! the stations value dataset
    H5::DataSet _grValDS; //! the grid value dataset
    H5::DataSet _grErrDS; //! the interpolation error dataset

    H5::DataSpace _st_dspace; //! dataspace for stations
    H5::DataSpace _gr_dspace; //! dataspace for grid
    H5::DataSpace _tm_dspace; //! dataspace for time

    H5::DataSet _yrDS; //! the years array dataset
    H5::DataSet _mnDS; //! the month array dataset
    H5::DataSet _dyDS; //! the days array dataset
    H5::DataSet _hrDS; //! the hours array dataset

    float _data_scale_factor;
    float _data_fill_value;
    int _time_fill_value;
    float _detection_limit;
    int _compression;
    bool _write_hours;

    void _createGroups(void);
    void _createDataSets(const rio::config& cnf);
    void _setAttributes(const rio::config& cnf);
    void _initStationInfo(const std::shared_ptr<rio::network> n);
    void _initGridInfo(const std::shared_ptr<rio::grid> g);

    size_t _write_count;

private:
    // some helper functions
    // could use some templates here, but runs into all sorts of rubbish.. so pragmatic for now...
    void seth5attr(H5::H5Object& l, const char* name, double value);
    void seth5attr(H5::H5Object& l, const char* name, float value);
    void seth5attr(H5::H5Object& l, const char* name, int value);
    void seth5attr(H5::H5Object& l, const char* name, std::string value);

    void writeVector(H5::H5Location& grp, const char* name, const std::vector<double>& x);
    void writeVector(H5::H5Location& grp, const char* name, const std::vector<float>& x);
    void writeVector(H5::H5Location& grp, const char* name, const std::vector<int>& x);
    void writeVector(H5::H5Location& grp, const char* name, const std::vector<std::string>& str);

    template <typename data_type>
    void writeField(H5::DataSet& ds, const data_type* vals);
};

}
