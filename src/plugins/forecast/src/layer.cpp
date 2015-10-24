#include <string>

#include "layer.h"

int _parseMatrix( Eigen::MatrixXd &m, const std::string &str ) {
  std::stringstream ss( str );
  int n = 0;
  for( int i=0; i<m.rows(); i++ ) {
    for ( int j=0; j<m.cols(); j++ ) {
      if( (ss>>m(i,j)).fail() ) break;
      n++;
    }
  }
  return n;
}

int _parseMatrix( Eigen::VectorXd &m, const std::string &str ) {
  std::stringstream ss( str );
  int n = 0;
  for( int i=0; i<m.size(); i++ ) {
    if( (ss>>m(i)).fail() ) break;
    n++;
  }
  return n;
}

namespace nnet {

  layer::layer( TiXmlElement *cnf, int ninput ) {

    int size;
    std::string mat_str;

    if ( ! cnf ) throw "invalid pointer to cnf";
    if ( cnf->QueryIntAttribute( "size", &size ) != TIXML_SUCCESS ) throw "cannot find layer size";
    if ( cnf->QueryStringAttribute( "transfcn", &_fcnName ) != TIXML_SUCCESS ) 
      throw "cannot find transfer function";

    cnf->QueryStringAttribute( "name", &_Name );

    // allocate vectors
    _LW = Eigen::MatrixXd::Zero( size, ninput ); // layer weights
    _B  = Eigen::VectorXd::Zero( size );      // layer biasses
    _a  = Eigen::VectorXd::Zero( size );         // layer values

    // set the transfer function, a bit heavy here but what the hell
    if ( this->fcnName() == "purelin" ) {
      _fcn = std::ptr_fun( nnet::purelin );
    } else if ( this->fcnName() == "logsig" ) {
      _fcn = std::ptr_fun( nnet::logsig );
    } else if ( this->fcnName() == "tansig" ) {
      _fcn = std::ptr_fun( nnet::tansig );
    } else if ( this->fcnName() == "hardlim" ) {
      _fcn = std::ptr_fun( nnet::hardlim );
    } else if ( this->fcnName() == "hardlims" ) {
      _fcn = std::ptr_fun( nnet::hardlims );
    } else if ( this->fcnName() == "poslin" ) {
      _fcn = std::ptr_fun( nnet::poslin );
    } else if ( this->fcnName() == "radbas" ) {
      _fcn = std::ptr_fun( nnet::radbas );
    } else if ( this->fcnName() == "satlin" ) {
      _fcn = std::ptr_fun( nnet::satlin );
    } else if ( this->fcnName() == "satlins" ) {
      _fcn = std::ptr_fun( nnet::satlins );
    } else if ( this->fcnName() == "tribas" ) {
      _fcn = std::ptr_fun( nnet::tribas );
    } else 
      throw "unknown transfer function requested";
    
    // read the layer weights and biases
    // this assumes we are reading the layer weight matrix row-wise from the text element
    if ( ! cnf->FirstChildElement( "weights" ) ) throw "no weights defined in layer";
    mat_str = cnf->FirstChildElement( "weights" )->GetText();
    if ( _parseMatrix( _LW, mat_str ) != _LW.rows()*_LW.cols() ) throw "error reading layer weights";
    
    if ( ! cnf->FirstChildElement( "bias" ) ) throw "no biasses defined in layer";
    mat_str = cnf->FirstChildElement( "bias" )->GetText();
    _parseMatrix( _B, mat_str );
  }
  
  layer::~layer(){
    // clear memory for the matrices
    _LW.resize(0,0);
    _B.resize(0);
    _a.resize(0);
  }

  void layer::compute( const Eigen::VectorXd &input ) {
    
    if ( input.size() != this->inputSize() ) 
      throw "invalid input size in layer::compute";
    
    // vectorized matrix product with weights & add bias
    _a = _LW * input + _B;  

    // apply transfer function element wise
    // cannot do this in place because of Eigen references
    // http://stackoverflow.com/questions/19624632/how-do-i-in-place-modify-each-element-of-a-1d-array
    _a = _a.unaryExpr( _fcn );

    return;
  }
  
  std::ostream& operator<<(std::ostream &os, const layer& lyr ) {
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

};
