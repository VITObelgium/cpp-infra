#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include <spdlog/fmt/ostr.h>

#include "feedforwardnet.h"
#include "infra/configdocument.h"
#include "infra/string.h"

namespace nnet {

using namespace infra;

namespace {

std::unique_ptr<nnet::scaler> createScaler(const infra::ConfigNode& config, int size)
{
    // get scaler type
    if (!config) {
        return std::make_unique<nnet::mapdummy>();
    }

    auto type_str = str::lowercase(config.attribute("type"));
    if (type_str.empty()) {
        throw std::runtime_error("cannot retrieve scaler type");
    }

    if (type_str == "none") {
        return std::make_unique<nnet::mapdummy>();
    } else if (type_str == "mapstd") {
        return std::make_unique<nnet::mapstd>(config, size); // MAPSTD SCALER
    } else if (type_str == "mapminmax") {
        return std::make_unique<nnet::mapminmax>(config, size); // MAPMINMAX SCALER
    } else {
        throw "unknown scaler detected";
    }
}
}

feedforwardnet::feedforwardnet(const infra::ConfigNode& config)
: _logger("feedforwardnet")
{
    if (!config) {
        return;
    }

    if (config.name() != "feedforwardnet") {
        throw std::runtime_error("wrong root element, expecting feedforwardnet");
    }

    // ---------------
    // get input layer
    // ---------------
    auto pLyr = config.child("input");
    if (!pLyr) {
        throw std::runtime_error("cannot find input specification");
    }

    auto sz = pLyr.attribute<int>("size");
    if (!sz.has_value()) {
        throw std::runtime_error("cannot retrieve input size");
    }

    // resizing input
    _input.resize(sz.value());

    // read input normalisation : scaler
    auto pScal = pLyr.child("scaler");
    if (pScal) {
        _inputScaler = createScaler(pScal, sz.value());
    }

    // --------------------
    // hidden/output layers
    // --------------------
    // loop over layers : output/hidden : they have weights
    auto ninputs = sz.value(); // set input size for first layer to be the nn input size
    for (auto lyr : config.children("layer")) {
        _layers.emplace_back(new nnet::layer(lyr, ninputs));

        // set the input size for the next layer
        ninputs = _layers.back()->size();
    }

    // --------------------
    // output layer scaling
    // --------------------
    pLyr = config.child("output");
    if (!pLyr) {
        throw std::runtime_error("cannot find output specification");
    }

    sz = pLyr.attribute<int>("size");
    if (!sz.has_value()) {
        throw "cannot retrieve output size";
    }

    // resizing input
    _output.resize(sz.value());

    // read input normalisation : scaler
    pScal = pLyr.child("scaler");
    if (pScal) {
        _outputScaler = createScaler(pScal, sz.value());
    }
}

int feedforwardnet::sim(const Eigen::VectorXd& in)
{
    return sim(in.data());
}

// here we go to implement the workhorse routines...
int feedforwardnet::sim(const double* inp)
{
    // copy over the input values
    for (unsigned int i = 0; i < this->inputSize(); i++)
        _input(i) = inp[i];

    _logger->trace("sim: network input: {}", _input);

    // apply the input scaler
    _inputScaler->apply(_input);
    _logger->trace("sim: scaled input: {}", _input);

    nnet::layer* prev_layer = nullptr;
    for (auto it = _layers.begin(); it != _layers.end(); ++it) {
        if (it == _layers.begin()) {
            (*it)->compute(_input);
        } else {
            (*it)->compute(prev_layer->valuesAsVector());
        }

        _logger->trace("sim: layer values: {}", (*it)->valuesAsVector());
        prev_layer = it->get();
    }

    // set output to output of last layer
    _output = _layers.back()->valuesAsVector();
    _logger->trace("sim: scaled output: {}", _output);

    // apply the reverse output scaler
    _outputScaler->reverse(_output); // --> need to make this public
    _logger->trace("sim: output: {}", _output);

    return 0;
}

std::ostream& operator<<(std::ostream& os, const feedforwardnet& net)
{
    os << "FEEDFORWARDNET" << std::endl;
    os << "INPUT" << std::endl;
    os << " - size   : " << net._input.size() << std::endl;
    os << " - scaler : " << net._inputScaler->name() << std::endl;
    os << "LAYERS" << std::endl;
    for (auto& layer : net._layers) {
        os << *layer << std::endl;
    }
    os << "OUTPUT" << std::endl;
    os << " - size   : " << net._output.size() << std::endl;
    os << " - scaler : " << net._outputScaler->name() << std::endl;

    return os;
}

} // namespace nnet
