#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include <spdlog/fmt/ostr.h>

#include "feedforwardnet.h"

namespace nnet
{

namespace
{

std::unique_ptr<nnet::scaler> createScaler(TiXmlElement* pScal, int size)
{
    // get scaler type
    if (!pScal)
    {
        return std::make_unique<nnet::mapdummy>();
    }

    std::string type_str;
    if (pScal->QueryStringAttribute("type", &type_str) != TIXML_SUCCESS)
        throw "cannot retrieve scaler type";
    std::transform(type_str.begin(), type_str.end(), type_str.begin(), ::tolower);
    if (type_str == "none")
    {
        return std::make_unique<nnet::mapdummy>();
    }
    else if (type_str == "mapstd")
    {
        return std::make_unique<nnet::mapstd>(pScal, size); // MAPSTD SCALER
    }
    else if (type_str == "mapminmax")
    {
        return std::make_unique<nnet::mapminmax>(pScal, size); // MAPMINMAX SCALER
    }
    else
    {
        throw "unknown scaler detected";
    }
}

}

feedforwardnet::feedforwardnet(TiXmlElement* cnf)
: _logger("feedforwardnet")
{
    std::string type_str;
    TiXmlElement *pLyr, *pScal;
    int sz, ninputs;

    if (cnf) {
        if (cnf->ValueStr() != "feedforwardnet") throw "wrong root element, expecting feedforwardnet";

        // ---------------
        // get input layer
        // ---------------
        pLyr = cnf->FirstChildElement("input");
        if (!pLyr) throw "cannot find input specification";
        if (pLyr->QueryIntAttribute("size", &sz) != TIXML_SUCCESS)
            throw "cannot retrieve input size";

        // resizing input
        _input.resize(sz);

        // read input normalisation : scaler
        pScal = pLyr->FirstChildElement("scaler");
        if (pScal)
        {
            _inputScaler = createScaler(pScal, sz);
        }

        // --------------------
        // hidden/output layers
        // --------------------
        // loop over layers : output/hidden : they have weights
        ninputs = sz; // set input size for first layer to be the nn input size
        pLyr    = cnf->FirstChildElement("layer");
        while (pLyr)
        {
            _layers.emplace_back(new nnet::layer(pLyr, ninputs));

            // set the input size for the next layer
            ninputs = _layers.back()->size();
            pLyr    = pLyr->NextSiblingElement("layer");
        }

        // --------------------
        // output layer scaling
        // --------------------
        pLyr = cnf->FirstChildElement("output");
        if (!pLyr) throw "cannot find output specification";
        if (pLyr->QueryIntAttribute("size", &sz) != TIXML_SUCCESS)
            throw "cannot retrieve output size";

        // resizing input
        _output.resize(sz);

        // read input normalisation : scaler
        pScal = pLyr->FirstChildElement("scaler");
        if (pScal)
        {
            _outputScaler = createScaler(pScal, sz);
        }
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
        _input(i)       = inp[i];

    _logger->trace("sim: network input: {}", _input);

    // apply the input scaler
    _inputScaler->apply(_input);
    _logger->trace("sim: scaled input: {}",  _input);

    nnet::layer* prev_layer = nullptr;
    for (auto it = _layers.begin(); it != _layers.end(); ++it)
    {
        if (it == _layers.begin())
        {
            (*it)->compute(_input);
        }
        else
        {
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
    for (auto& layer : net._layers)
    {
        os << *layer << std::endl;
    }
    os << "OUTPUT" << std::endl;
    os << " - size   : " << net._output.size() << std::endl;
    os << " - scaler : " << net._outputScaler->name() << std::endl;

    return os;
}

} // namespace nnet
