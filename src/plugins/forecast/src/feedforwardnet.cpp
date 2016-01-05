#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>

#include "feedforwardnet.h"

namespace nnet {

  feedforwardnet::feedforwardnet( TiXmlElement *cnf ) {

    std::string type_str;
    TiXmlElement *pLyr, *pScal;
    int sz, ninputs;
    
    this->verbose = false;

    if ( cnf ) {
      if ( cnf->ValueStr() != "feedforwardnet" ) throw "wrong root element, expecting feedforwardnet";
      
      // ---------------
      // get input layer
      // ---------------
      pLyr = cnf->FirstChildElement( "input" );
      if ( ! pLyr ) throw "cannot find input specification";
      if ( pLyr->QueryIntAttribute( "size", &sz ) != TIXML_SUCCESS ) 
	throw "cannot retrieve input size"; 
      
      // resizing input
      _input.resize(sz);
      
      // read input normalisation : scaler
      pScal = pLyr->FirstChildElement("scaler"); 
      if ( pScal )  _inputScaler = _setScaler( pScal, sz );
      
      // --------------------
      // hidden/output layers
      // --------------------
      // loop over layers : output/hidden : they have weights
      ninputs = sz; // set input size for first layer to be the nn input size
      pLyr = cnf->FirstChildElement( "layer" );
      while ( pLyr ) {
	_layers.push_back( new nnet::layer( pLyr, ninputs ) );
	
	// set the input size for the next layer
	ninputs = _layers.back()->size();
	pLyr = pLyr->NextSiblingElement( "layer" );
      }

      // --------------------
      // output layer scaling
      // --------------------
      pLyr = cnf->FirstChildElement( "output" );
      if ( ! pLyr ) throw "cannot find output specification";
      if ( pLyr->QueryIntAttribute( "size", &sz ) != TIXML_SUCCESS ) 
	throw "cannot retrieve output size"; 
      
      // resizing input
      _output.resize(sz);
      
      // read input normalisation : scaler
      pScal = pLyr->FirstChildElement("scaler"); 
      if ( pScal )  _outputScaler = _setScaler( pScal, sz );
      
    } 
    
  }
  
  // destructor
  feedforwardnet::~feedforwardnet(){
    if ( _inputScaler ) delete _inputScaler; 
    if ( _outputScaler ) delete _outputScaler;
    
    _input.resize(0);   // the input sample
    _output.resize(0);  // the output sample
    
    // cleanup the layers
    for ( std::vector<nnet::layer*>::iterator it = _layers.begin();
	  it != _layers.end();
	  ++it ) delete (*it);
  }

  nnet::scaler *feedforwardnet::_setScaler( TiXmlElement *pScal, int size ) {
    
    std::string type_str;
    
    // get scaler type
    if ( pScal ) {
      if ( pScal->QueryStringAttribute( "type", &type_str ) != TIXML_SUCCESS ) 
	throw "cannot retrieve scaler type";
      std::transform( type_str.begin(), type_str.end(), type_str.begin(), ::tolower );
      if ( type_str == "none" ) {
	return new nnet::mapdummy();
      } else if ( type_str == "mapstd" ) {
	return new nnet::mapstd( pScal, size ); // MAPSTD SCALER
      } else if ( type_str == "mapminmax" ) {
	return new nnet::mapminmax( pScal, size ); // MAPMINMAX SCALER
      } else throw "unknown scaler detected";
    } 
    
    // otherwise return empty scaler  
    return new nnet::mapdummy(); 
  }
  
  int feedforwardnet::sim( const Eigen::VectorXd& in ) {
    return this->sim( in.data() );
  }

  // here we go to implement the workhorse routines...
  int feedforwardnet::sim( const double *inp ) {
    
    // copy over the input values
    for( unsigned int i = 0; i<this->inputSize(); i++ ) _input(i) = inp[i];

    if ( this->verbose ) std::cout << "[feedforwardnet::sim] network input : " 
				   << std::endl << _input << std::endl;

    // apply the input scaler
    _inputScaler->apply( _input );

    if ( this->verbose ) std::cout << "[feedforwardnet::sim] scaled input : " 
				   << std::endl << _input << std::endl;

    nnet::layer *prev_layer = NULL;
    for( std::vector<nnet::layer*>::const_iterator it =_layers.begin();
	 it != _layers.end(); 
	 ++it ) {
      if ( it == _layers.begin() ) {
	(*it)->compute( _input ); 
      } else {
	(*it)->compute( prev_layer->valuesAsVector() );
      }

      if ( this->verbose ) std::cout << "[feedforwardnet::sim] layer values : " 
				     << std::endl 
				     << (*it)->valuesAsVector() << std::endl;

      prev_layer = *it;
    }

    // set output to output of last layer
    _output = _layers.back()->valuesAsVector();

    if ( this->verbose ) std::cout << "[feedforwardnet::sim] scaled output : " 
				   << std::endl << _output << std::endl;

    // apply the reverse output scaler
    _outputScaler->reverse( _output ); // --> need to make this public

    if ( this->verbose ) std::cout << "[feedforwardnet::sim] output : " 
				   << std::endl << _output << std::endl;

    return 0;
  }
  
  std::ostream& operator<<(std::ostream &os, const feedforwardnet& net ) {
    os << "FEEDFORWARDNET" << std::endl;
    os << "INPUT" << std::endl;
    os << " - size   : " << net._input.size() << std::endl;
    os << " - scaler : " << net._inputScaler->name() << std::endl;
    os << "LAYERS" << std::endl;
    for( std::vector<nnet::layer*>::const_iterator it = net._layers.begin(); 
	 it != net._layers.end(); 
	 ++it ) {
      os << **it << std::endl;
    }
    os << "OUTPUT" << std::endl;
    os << " - size   : " << net._output.size() << std::endl;
    os << " - scaler : " << net._outputScaler->name() << std::endl;

    return os;
  }
  
  
} // namespace nnet
