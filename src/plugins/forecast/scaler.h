#ifndef __SCALER_H
#define __SCALER_H

#include <Eigen/Dense>

namespace inf {
class ConfigNode;
}

namespace nnet {

// class to scale from original data x to normalised data y
// base class implements an empty scaler, i.e. no normalisation
// derived classes implement scaling functionality
class scaler
{
public:
    scaler();
    virtual ~scaler();

    //use Eigen Refs for passing by reference, see
    virtual int apply(Eigen::Ref<Eigen::VectorXd> x)   = 0;
    virtual int reverse(Eigen::Ref<Eigen::VectorXd> y) = 0;

    std::string name(void)
    {
        return _name;
    }

protected:
    std::string _name;
};

class mapdummy : public scaler
{
public:
    mapdummy();
    ~mapdummy();

    int apply(Eigen::Ref<Eigen::VectorXd> x);
    int reverse(Eigen::Ref<Eigen::VectorXd> y);
};

// scaler to : map mean/std to 0/1
class mapstd : public scaler
{
public:
    mapstd(const inf::ConfigNode& config, int size, double ymean = 0, double ystd = 1);
    ~mapstd();

    int apply(Eigen::Ref<Eigen::VectorXd> x);
    int reverse(Eigen::Ref<Eigen::VectorXd> y);

private:
    double _ymean;
    double _ystd;
    Eigen::VectorXd _xmean;
    Eigen::VectorXd _xstd;
};

// scaler to map the min and max to [-1 1]
class mapminmax : public scaler
{
public:
    mapminmax(const inf::ConfigNode& config, int size, double ymin = -1, double ymax = 1);
    ~mapminmax();

    int apply(Eigen::Ref<Eigen::VectorXd> x);
    int reverse(Eigen::Ref<Eigen::VectorXd> y);

private:
    double _ymin;
    double _ymax;
    Eigen::VectorXd _xmin;
    Eigen::VectorXd _xmax;
};
};

#endif /* #ifndef __SCALER_H */
