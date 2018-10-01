#include <string>

#include "infra/configdocument.h"
#include "layer.h"

int _parseMatrix(Eigen::MatrixXd& m, const std::string& str)
{
    std::stringstream ss(str);
    int n = 0;
    for (int i = 0; i < m.rows(); i++) {
        for (int j = 0; j < m.cols(); j++) {
            if ((ss >> m(i, j)).fail()) break;
            n++;
        }
    }
    return n;
}

int _parseMatrix(Eigen::VectorXd& m, const std::string& str)
{
    std::stringstream ss(str);
    int n = 0;
    for (int i = 0; i < m.size(); i++) {
        if ((ss >> m(i)).fail()) break;
        n++;
    }
    return n;
}

namespace nnet {

using namespace inf;

layer::layer(const ConfigNode& config, int ninput)
: _fcn(nullptr)
{
    if (!config) {
        throw std::runtime_error("invalid layer config");
    }

    auto size = config.attribute<int>("size");
    if (!size.has_value()) {
        throw std::runtime_error("cannot find layer size");
    }

    _fcnName = std::string(config.attribute("transfcn"));
    if (_fcnName.empty()) {
        throw std::runtime_error("cannot find transfer function");
    }

    _Name = config.attribute("name");

    // allocate vectors
    _LW = Eigen::MatrixXd::Zero(*size, ninput); // layer weights
    _B  = Eigen::VectorXd::Zero(*size);         // layer biasses
    _a  = Eigen::VectorXd::Zero(*size);         // layer values

    // set the transfer function, a bit heavy here but what the hell
    if (this->fcnName() == "purelin") {
        _fcn = nnet::purelin;
    } else if (this->fcnName() == "logsig") {
        _fcn = nnet::logsig;
    } else if (this->fcnName() == "tansig") {
        _fcn = nnet::tansig;
    } else if (this->fcnName() == "hardlim") {
        _fcn = nnet::hardlim;
    } else if (this->fcnName() == "hardlims") {
        _fcn = nnet::hardlims;
    } else if (this->fcnName() == "poslin") {
        _fcn = nnet::poslin;
    } else if (this->fcnName() == "radbas") {
        _fcn = nnet::radbas;
    } else if (this->fcnName() == "satlin") {
        _fcn = nnet::satlin;
    } else if (this->fcnName() == "satlins") {
        _fcn = nnet::satlins;
    } else if (this->fcnName() == "tribas") {
        _fcn = nnet::tribas;
    } else
        throw std::runtime_error("unknown transfer function requested");

    // read the layer weights and biases
    // this assumes we are reading the layer weight matrix row-wise from the text element
    auto weightsNode = config.child("weights");
    if (!weightsNode) {
        throw std::runtime_error("no weights defined in layer");
    }

    auto mat_str = std::string(weightsNode.value());
    if (_parseMatrix(_LW, mat_str) != _LW.rows() * _LW.cols()) {
        throw std::runtime_error("error reading layer weights");
    }

    auto biasNode = config.child("bias");
    if (!biasNode) {
        throw std::runtime_error("no biasses defined in layer");
    }

    mat_str = std::string(biasNode.value());
    _parseMatrix(_B, mat_str);
}

layer::~layer()
{
    // clear memory for the matrices
    _LW.resize(0, 0);
    _B.resize(0);
    _a.resize(0);
}

void layer::compute(const Eigen::VectorXd& input)
{
    if (input.size() != this->inputSize())
        throw "invalid input size in layer::compute";

    // vectorized matrix product with weights & add bias
    _a = _LW * input + _B;

    // apply transfer function element wise
    // cannot do this in place because of Eigen references
    // http://stackoverflow.com/questions/19624632/how-do-i-in-place-modify-each-element-of-a-1d-array
    _a = _a.unaryExpr(_fcn);

    return;
}

std::ostream& operator<<(std::ostream& os, const layer& lyr)
{
    os << "layer: " << lyr._Name << std::endl;
    os << "  input size      : " << lyr._LW.cols() << std::endl;
    os << "  output size     : " << lyr._LW.rows() << std::endl;
    os << "  transfer fcn    : " << lyr._fcnName << std::endl;
    os << "  LW : " << std::endl;
    os << lyr._LW << std::endl;
    os << "  BIAS : " << std::endl;
    os << lyr._B << std::endl;
    os << "  VALUES : " << std::endl;
    os << lyr._a << std::endl;
    return os;
}

}; // namespace nnet
