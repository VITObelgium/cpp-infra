#pragma once

#include "outputhandler.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace rio {

/**
   A GeoJSON writer, making it extremely easy to generate maps with python/geopandas
   
   import geopandas as gdp
   gpd.read_file( 'rio_20150102T020000.json' ).plot( 'concentration' )

   et voila :)

 */
class jsonwriter : public outputhandler
{
public:
    jsonwriter(const inf::XmlNode& cnf);
    ~jsonwriter();

    void init(const rio::config& cnf,
        const std::shared_ptr<rio::network> net,
        const std::shared_ptr<rio::grid> grid) override;

    void write(const boost::posix_time::ptime& curr_time,
        const std::unordered_map<std::string, double>& obs,
        const Eigen::VectorXd& values,
        const Eigen::VectorXd& uncert) override;

    void close(void);

private:
    std::string _pattern; //! output filename pattern
    std::string _polName; //! pollutant name

    std::shared_ptr<rio::network> _net;
    std::shared_ptr<rio::grid> _grid;

    boost::posix_time::time_duration _dt;
};

}
