#ifndef __FFNET_H
#define __FFNET_H

#include <iostream>
#include <math.h>
#include <vector>

#include <Eigen/Dense>

#include "layer.h"
#include "scaler.h"

namespace infra {
class ConfigNode;
}

namespace nnet {

class feedforwardnet
{
public:
    feedforwardnet(const infra::ConfigNode& config);

    int sim(const double* input);
    int sim(const Eigen::VectorXd& in);

    friend std::ostream& operator<<(std::ostream& os, const feedforwardnet& net);

    unsigned int inputSize(void)
    {
        return static_cast<unsigned int>(_input.size());
    }

    unsigned int outputSize(void)
    {
        return static_cast<int>(_output.size());
    }

    void getOutput(double** x)
    {
        *x = _output.data();
    }
    void getOutput(Eigen::VectorXd& out)
    {
        out = _output;
    }

private:
    Eigen::VectorXd _input;                            // the input sample
    Eigen::VectorXd _output;                           // the output sample
    std::vector<std::unique_ptr<nnet::layer>> _layers; // the hidden/output layers

    std::unique_ptr<nnet::scaler> _inputScaler;
    std::unique_ptr<nnet::scaler> _outputScaler;
};

}; // namespace nnet

#endif /* #ifndef __FFNET_H */
