#include <cmath>

#include "idw_ipol.hpp"

namespace rio {

idw_ipol::idw_ipol(const config& cf, const std::shared_ptr<rio::network>& net)
: mapper(cf, net)
, _p(1.0)
{
    _name = "IDW";

    try {
        _p = _opts.get<double>("options.power");
    } catch (...) {
        throw std::runtime_error("error in getting the idw power...");
    }
    std::cout << " IDW power set to " << _p << std::endl;
}

idw_ipol::~idw_ipol()
{
}

void idw_ipol::run(Eigen::VectorXd& values,
    Eigen::VectorXd& uncert,
    boost::posix_time::ptime /*tstart*/,
    const std::unordered_map<std::string, double>& obs,
    const std::shared_ptr<rio::grid>& g) const
{
    // get the stations x and y values and observations
    Eigen::VectorXd xi(obs.size());
    Eigen::VectorXd yi(obs.size());
    Eigen::VectorXd V(obs.size());

    size_t i = 0;
    for (const auto& it : obs) {
        std::shared_ptr<rio::station const> st = _net->get(it.first);
        if (st == nullptr) throw std::runtime_error("internal error, station not found");

        xi[i] = st->x();
        yi[i] = st->y();
        V[i]  = it.second;
        i++;
    }

    //for ( int i = 0; i < xi.size(); i++ )
    //  std::cout << xi[i] << " " << yi[i] << " " << V[i] << std::endl;

    // compute distance matrix, partly vectorized
    Eigen::MatrixXd w(xi.size(), g->size());
    for (int i = 0; i < g->size(); i++) {
        w.col(i) = sqrt(pow(xi.array() - g->cells()[i].cx(), 2.) +
                        pow(yi.array() - g->cells()[i].cy(), 2.));
    }

    // write out distance matrix
    /*
  std::ofstream ofs( "rmat.txt" );
  ofs << std::setprecision(15) << w;
  ofs.close();
  */
    // compute inverse & power of distance matrix
    w = w.array().pow(-fabs(_p));

    /*
  std::ofstream ofs2( "rmat_inv.txt" );
  ofs2 << std::setprecision(15) << w;
  ofs2.close();
  */

    // normalize each row to sum over the columns
    // a row corresponds to the weights for a single grid location
    Eigen::VectorXd sum = w.rowwise().sum();
    w.array().colwise() /= sum.array();

    // matrix product
    values = w.transpose() * V;
    uncert.setConstant(values.size(), 0.);

    // postprocess
    for (int i = 0; i < values.size(); i++) {
        if (std::isnan(values(i)) ||
            std::isinf(values(i))) {
            values(i) = _missing_value;
            uncert(i) = _missing_value;
        }

        // apply detection limit
        if (values(i) < _detection_limit) values(i) = _detection_limit;
    }

    return;
}

}
