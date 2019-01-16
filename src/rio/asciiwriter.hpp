#pragma once

#include "outputhandler.hpp"

namespace rio {

class asciiwriter : public outputhandler
{
public:
    asciiwriter(const inf::XmlNode& cnf);
    ~asciiwriter();

    void init(const rio::config& cnf,
        const std::shared_ptr<rio::network> net,
        const std::shared_ptr<rio::grid> grid) override;

    void write(const boost::posix_time::ptime& curr_time,
        const std::unordered_map<std::string, double>& obs,
        const Eigen::VectorXd& values,
        const Eigen::VectorXd& uncert) override;

    void close() override;

private:
    std::string _pattern; //! output filename pattern
    char _fmt[100];       //!
    char _fs;             //! field separator
    int _pr;              //! output precision

    std::shared_ptr<rio::network> _net;
    std::shared_ptr<rio::grid> _grid;
};

}
