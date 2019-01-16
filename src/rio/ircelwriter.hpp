#pragma once

#include <boost/date_time.hpp>
#include <locale>
#include <unordered_map>

#include "outputhandler.hpp"

namespace rio {

class ircelwriter : public outputhandler
{
public:
    ircelwriter(const inf::XmlNode& cnf);
    virtual ~ircelwriter();

    void init(const rio::config& cnf,
        const std::shared_ptr<rio::network> net,
        const std::shared_ptr<rio::grid> grid) override;

    void write(const boost::posix_time::ptime& curr_time,
        const std::unordered_map<std::string, double>& obs,
        const Eigen::VectorXd& values,
        const Eigen::VectorXd& uncert) override;

    void close() override;

private:
    void write_buffers(const std::string& fname);

protected:
    const std::unordered_map<std::string, std::string> _saroad_codes;
    std::string _saroad;
    const std::locale _dateFmt;

    std::string _pattern; //! output filename pattern

    std::string _fname; //! filename

    boost::gregorian::date _curr_day;

    std::shared_ptr<rio::network> _net;
    std::shared_ptr<rio::grid> _grid;

    // buffer
    Eigen::MatrixXd _valbuf;
    Eigen::MatrixXd _uncbuf;
};

}
