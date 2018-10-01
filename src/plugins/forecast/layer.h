#ifndef __NNLAYER_H
#define __NNLAYER_H

#include <functional>

#include <Eigen/Dense>

#include "transfcn.h"

namespace inf {
class ConfigNode;
}

namespace nnet {

// base class for a neural network layer
class layer
{
public:
    layer(const inf::ConfigNode& config, int ninputs);
    virtual ~layer();

    friend std::ostream& operator<<(std::ostream& os, const layer& lyr);

    int size()
    {
        return static_cast<int>(_a.size());
    }
    std::string name()
    {
        return _Name;
    }
    double* values()
    {
        return _a.data();
    } // the .data() method contains a const pointer to the scalar data
    Eigen::VectorXd& valuesAsVector()
    {
        return _a;
    }

    int inputSize()
    {
        return static_cast<int>(_LW.cols());
    }
    int outputSize()
    {
        return static_cast<int>(_LW.rows());
    }

    std::string fcnName()
    {
        return _fcnName;
    } // transfer function

    // propagates an input vector through this layer
    void compute(const Eigen::VectorXd& input);

    // previous and next layers
    layer* prev;
    layer* next;

private:
    std::string _Name; // layer name

    Eigen::VectorXd _a;  // layer values
    Eigen::MatrixXd _LW; // layer weights : from previous to this layer
    Eigen::VectorXd _B;  // layer bias vector

    // the transfer function :
    // might think of encapsulating this in a class, but a bit overkill
    std::function<double(double)> _fcn; // function pointer
    std::string _fcnName;               // name
};

}; // namespace nnet

#endif /* #ifndef __NNLAYER_H */
