#include <iostream>
#include <sstream>
#include <string>

#include "scaler.h"


// little helper routine
int _parseArray( Eigen::VectorXd &v, const std::string &str ) {
  std::stringstream ss( str );
  int n = 0;
  for( int i=0; i<v.size(); i++ ) {
    if( (ss>>v(i)).fail() ) break;
    n++;
  }
  return n;
}

namespace nnet {

  scaler::scaler() {
  }

  scaler::~scaler(){
  }


  // DUMMY SCALER
  mapdummy::mapdummy( ) : 
    scaler() {
    _name = "mapdummy";
  }
  mapdummy::~mapdummy(){
  }
  int mapdummy::apply( Eigen::Ref<Eigen::VectorXd> x ) {  return 0; }
  int mapdummy::reverse( Eigen::Ref<Eigen::VectorXd> y ) { return 0; }


  // MAP STD IMPLEMENTATION
  mapstd::mapstd( TiXmlElement *cnf, int size, double ymean, double ystd ) : 
    scaler(), _ymean(ymean), _ystd(ystd) {
    
    std::string xmean_str, xstd_str;

    _name = "mapstd";

    if ( ! cnf->FirstChildElement( "xmean" ) ) throw "cannot find xmean element";
    xmean_str = cnf->FirstChildElement( "xmean" )->GetText();
    if ( ! cnf->FirstChildElement( "xstd" ) ) throw "cannot find xstd element";
    xstd_str = cnf->FirstChildElement( "xstd" )->GetText();

    // resize the matrices beforehand
    _xmean.resize(size);
    _xstd.resize(size);

    if ( _parseArray( _xmean, xmean_str ) != size ) throw "error parsing xmean";
    if ( _parseArray( _xstd, xstd_str ) != size ) throw "error parsing xstd";

    //std::cout << "xmean: " << std::endl << _xmean << std::endl;
    //std::cout << "xstd: "  << std::endl << _xstd  << std::endl;
  }
    

  mapstd::~mapstd(){
    // clear the memory
    _xmean.resize(0);
    _xstd.resize(0);
  }

  int mapstd::apply( Eigen::Ref<Eigen::VectorXd> x ) {
    if ( x.size() != _xmean.size() ) return 1;

    x = (x.array()-_xmean.array())*(_ystd/_xstd.array()) + _ymean;

    //std::cout << "calculated x internallty" << std::endl;
    //std::cout << x << std::endl;
    
    return 0;
  }
  
  int mapstd::reverse( Eigen::Ref<Eigen::VectorXd> y ) {
    if ( y.size() != _xmean.size() ) return 1;
    y = (y.array()-_ymean)/(_ystd/_xstd.array()) + _xmean.array();
    return 0;
  }
  

  // MAP MINMAX IMPLEMENTATION
  mapminmax::mapminmax( TiXmlElement *cnf, int size, double ymin, double ymax ) : 
    scaler(), _ymin(ymin), _ymax(ymax) {

    std::string xmin_str, xmax_str;

    _name = "mapminmax";

    if ( ! cnf->FirstChildElement( "xmin" ) ) throw "cannot find xmin element";
    xmin_str = cnf->FirstChildElement( "xmin" )->GetText();
    if ( ! cnf->FirstChildElement( "xmax" ) ) throw "cannot find xmax element";
    xmax_str = cnf->FirstChildElement( "xmax" )->GetText();

    // resize the matrices beforehand
    _xmin.resize(size);
    _xmax.resize(size);

    if ( _parseArray( _xmin, xmin_str ) != size ) throw "error parsing xmin";
    if ( _parseArray( _xmax, xmax_str ) != size ) throw "error parsing xmax";

    //std::cout << "xmin: " << std::endl << _xmin << std::endl;
    //std::cout << "xmax: " << std::endl << _xmax  << std::endl;
  }
  
  mapminmax::~mapminmax(){
    // clear the memory
    _xmin.resize(0);
    _xmax.resize(0);
  }

  int mapminmax::apply( Eigen::Ref<Eigen::VectorXd> x ) {
    if ( x.size() != _xmin.size() ) return 1;
    x = (_ymax-_ymin)*(x.array()-_xmin.array())/(_xmax.array()-_xmin.array()) + _ymin;
    return 0;
  }

  int mapminmax::reverse( Eigen::Ref<Eigen::VectorXd> y ) {
    if ( y.size() != _xmin.size() ) return 1;
    y = (y.array()-_ymin)/(_ymax-_ymin)*(_xmax.array()-_xmin.array()) + _xmin.array();
    return 0;
  }
  

}; // namespace nnet
