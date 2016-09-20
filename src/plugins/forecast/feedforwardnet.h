#ifndef __FFNET_H
#define __FFNET_H

#include <iostream>
#include <vector>
#include <math.h>

#include <tinyxml.h>

#include <Eigen/Dense>

#include "scaler.h"
#include "layer.h"

namespace nnet {

  class feedforwardnet {
  public: 
    feedforwardnet( TiXmlElement *cnf );
    virtual ~feedforwardnet();
    
    int sim( const double *input );
    int sim( const Eigen::VectorXd& in );
    
    friend std::ostream& operator<<(std::ostream& os, const feedforwardnet& net );

    unsigned int inputSize( void ) { return static_cast<unsigned int>(_input.size()); }
    unsigned int outputSize( void ) { return static_cast<int>(_output.size()); }

    void getOutput( double **x ) { *x = _output.data(); }
    void getOutput( Eigen::VectorXd& out ) { out = _output; }

    bool verbose;

  private:
    Eigen::VectorXd           _input;   // the input sample
    Eigen::VectorXd           _output;  // the output sample
    std::vector<nnet::layer*> _layers;  // the hidden/output layers
    
    nnet::scaler *_inputScaler; 
    nnet::scaler *_outputScaler;

    nnet::scaler *_setScaler( TiXmlElement *el, int size );

  };
  
}; // namespace nnet

#endif /* #ifndef __FFNET_H */
