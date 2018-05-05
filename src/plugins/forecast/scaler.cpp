#include <iostream>
#include <sstream>
#include <string>

#include "infra/configdocument.h"
#include "scaler.h"

// little helper routine
int _parseArray(Eigen::VectorXd& v, std::string_view str)
{
    std::stringstream ss;
    ss.write(str.data(), str.size());
    int n = 0;
    for (int i = 0; i < v.size(); i++) {
        if ((ss >> v(i)).fail()) break;
        n++;
    }
    return n;
}

namespace nnet {

using namespace infra;

scaler::scaler()
{
}

scaler::~scaler()
{
}

// DUMMY SCALER
mapdummy::mapdummy()
: scaler()
{
    _name = "mapdummy";
}
mapdummy::~mapdummy()
{
}
int mapdummy::apply(Eigen::Ref<Eigen::VectorXd> x)
{
    return 0;
}
int mapdummy::reverse(Eigen::Ref<Eigen::VectorXd> y)
{
    return 0;
}

// MAP STD IMPLEMENTATION
mapstd::mapstd(const infra::ConfigNode& config, int size, double ymean, double ystd)
: scaler(), _ymean(ymean), _ystd(ystd)
{
    _name          = "mapstd";
    auto xmean_str = config.child("xmean").value();
    if (xmean_str.empty()) {
        throw std::runtime_error("cannot find xmean element");
    }

    auto xstd_str = config.child("xstd").value();
    if (xstd_str.empty()) {
        throw std::runtime_error("cannot find xstd element");
    }

    // resize the matrices beforehand
    _xmean.resize(size);
    _xstd.resize(size);

    if (_parseArray(_xmean, xmean_str) != size) throw "error parsing xmean";
    if (_parseArray(_xstd, xstd_str) != size) throw "error parsing xstd";

    //std::cout << "xmean: " << std::endl << _xmean << std::endl;
    //std::cout << "xstd: "  << std::endl << _xstd  << std::endl;
}

mapstd::~mapstd()
{
    // clear the memory
    _xmean.resize(0);
    _xstd.resize(0);
}

int mapstd::apply(Eigen::Ref<Eigen::VectorXd> x)
{
    if (x.size() != _xmean.size()) return 1;

    x = (x.array() - _xmean.array()) * (_ystd / _xstd.array()) + _ymean;

    //std::cout << "calculated x internallty" << std::endl;
    //std::cout << x << std::endl;

    return 0;
}

int mapstd::reverse(Eigen::Ref<Eigen::VectorXd> y)
{
    if (y.size() != _xmean.size()) return 1;
    y = (y.array() - _ymean) / (_ystd / _xstd.array()) + _xmean.array();
    return 0;
}

// MAP MINMAX IMPLEMENTATION
mapminmax::mapminmax(const infra::ConfigNode& config, int size, double ymin, double ymax)
: scaler(), _ymin(ymin), _ymax(ymax)
{
    _name = "mapminmax";

    auto xmin_str = config.child("xmin").value();
    if (xmin_str.empty()) {
        throw std::runtime_error("cannot find xmin element");
    }

    auto xmax_str = config.child("xmax").value();
    if (xmax_str.empty()) {
        throw std::runtime_error("cannot find xmax element");
    }

    // resize the matrices beforehand
    _xmin.resize(size);
    _xmax.resize(size);

    if (_parseArray(_xmin, xmin_str) != size) throw "error parsing xmin";
    if (_parseArray(_xmax, xmax_str) != size) throw "error parsing xmax";

    //std::cout << "xmin: " << std::endl << _xmin << std::endl;
    //std::cout << "xmax: " << std::endl << _xmax  << std::endl;
}

mapminmax::~mapminmax()
{
    // clear the memory
    _xmin.resize(0);
    _xmax.resize(0);
}

int mapminmax::apply(Eigen::Ref<Eigen::VectorXd> x)
{
    if (x.size() != _xmin.size()) return 1;
    x = (_ymax - _ymin) * (x.array() - _xmin.array()) / (_xmax.array() - _xmin.array()) + _ymin;
    return 0;
}

int mapminmax::reverse(Eigen::Ref<Eigen::VectorXd> y)
{
    if (y.size() != _xmin.size()) return 1;
    y = (y.array() - _ymin) / (_ymax - _ymin) * (_xmax.array() - _xmin.array()) + _xmin.array();
    return 0;
}

}; // namespace nnet
