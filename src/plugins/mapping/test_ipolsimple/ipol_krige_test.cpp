/*
  OPAQ kriging interpolation plugin
  
*/
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <map>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Cholesky>

#include "station.h"
#include "network.h"
#include "grid.h"

#define MAX(A,B) ( A > B ? A : B )

class SpatialCorrModel {
public:
  SpatialCorrModel( std::string type ) 
    : _p(NULL), _n(0) {

    /*
    // set correlation model
    if ( type == "exp" ) {
      _corrModel = std::ptr_fun( this->expModel );
    } else if ( type == "explin" ) {
      _corrModel = std::ptr_fun( this->explinModel );
    } else 
      throw "SpatialCorrModel:: unknown correlation model type";
    */
    _corrModelName = type;

    // set lower/upper limits for distance
    _dmin = 1e-3;
    _dmax = 99999999.;
  }

  ~SpatialCorrModel(){
  }
  
  // here we return : xs,ys : sample locations, x, y : grid locations
  // if ns = number of sample locations and ng = number of grid locations
  // we return a ng*ns matrix
  // 
  int corrmat( const Eigen::VectorXd &xs, const Eigen::VectorXd &ys,
	       const Eigen::VectorXd &x, const Eigen::VectorXd &y,
	       Eigen::MatrixXd &rho ) {
    
    int rows = x.size();
    int cols = xs.size();

    // operate only on the top left part of the matrix, thereby 
    // allowing a larger matrix to be passed
    std::cout << "expected rows in corrmat : " << rows << ", expected cols : " << cols
	      << ", rho argument, rows = " 
	      << rho.rows() << ", cols = " << rho.cols() 
	      << std::endl;
    
    // if we have a smaller matrix than the one given, we need to resize the matrix
    // otherwise we leave the dimensions as they are, we only fill the block(0,0,rows,cols)
    if ( ( rho.rows() < rows ) && ( rho.cols() < cols ) ) {
      rho.resize( rows,cols );
    } else if ( rho.rows() < rows ) {
      rho.resize( rows, rho.cols() ); // cols stays the same
      std::cout << "SpatialCorrModel::corrmat, warning : resizing only rows" << std::endl;
    } else if ( rho.cols() < cols ) {
      rho.resize( rho.rows(),cols ); // rows stays the same
      std::cout << "SpatialCorrModel::corrmat, warning : resizing only columns" << std::endl;
    }

    // first compute the distance matrix in rho, 
    // partly vectorized
    for( int i=0; i<cols; i++ ) {
      rho.block(0,i,rows,1) = sqrt( pow(x.array()-xs(i),2) + pow(y.array()-ys(i),2) );
    }
    
    // dump distance matrix
    std::ofstream ofs( "rmat_st.txt" );
    ofs << rho;
    ofs.close();


    // apply correlation model to distances in the rho matrix
    // rho.block(0,0,rows,cols) = rho.block(0,0,rows,cols).unaryExpr( _corrModel );

    double A   = 0.95;
    double tau = 4.;
    rho.block(0,0,rows,cols) =  A*exp( -rho.block(0,0,rows,cols).array()*1e-3 / tau );
    //      A*((-rho.block(0,0,rows,cols).array()/tau).exp());
      
  }
  /*  
  // correlation model types : only a function of distance r
  // simple exponentional model
  double expModel( double r ) { 
    
    if ( _n < 2 ) throw "SpatialCorrModel::expModel error, need at least 2 parameters";

    if ( ( _dmin > 0 ) && ( r < _low_limit ) ) return 1.; // corr is 1 below 1 m
    if ( ( _dmax > 0 ) && ( r > _dmax ) ) return expModel( _dmax );

    double A   = exp(_p[1]);
    double tau = -1/_p[0];

    return A*exp(-r/tau);
  } 

  double explinModel( double r ) { 

    if ( _n < 4 ) throw "SpatialCorrModel::explinModel error, need at least 4 parameters";

    if ( ( _dmin > 0 ) && ( r < _low_limit ) ) return 1.; // corr is 1 below 1 m
    if ( ( _dmax > 0 ) && ( r > _dmax ) ) return explinModel( _dmax );

    double A   = exp(_p[1]);
    double tau = -1/_p[0];

    double c_short = _p[2] * r + _p[3];
    double c_long  = A*exp(-r/tau);

    return MAX( 0., MAX( c_short, c_long ) );
  } 
  */
  double* getParams( void ) { return _p; }
  void setParams( double *p, int n ) {
    // (re-)alloc memory for parameters
    if ( ! _p ) {
      _p = new double[n];
      _n = n;
    } else if ( n != _n ) {
      std::cout << "SpatialCorrelationModel::warning:reallocing memory for parameters" << std::endl;
      delete [] _p;
      _p = new double[n];
      _n = n;
    }
    // set values
    for( int i=0; i<n; ++i ) _p[i] = p[i];
    return;
  }
  double getPar( int i ) {
    if ( i < _n ) return _p[i];
    throw "SpatialCorrelationModel Error : parameter index out of bounds";
    return NAN;
  }

private:
  double *_p; // parameter vector
  int     _n; // number of parameters in the correlation model
  bool    _have_pars; 

  double  _dmin; // correlations are set to 1 below this distance ( if > 0)
  double  _dmax; // correlations are kept constant above this distance (if > 0)

  std::pointer_to_unary_function<double,double> _corrModel; // correlation model function pointer
  std::string                                   _corrModelName; // name for corr model
};



// --> werk met Ref<> waarschijnlijk om die Map zaken goed te krijgen....
int ipol_krige( SpatialCorrModel *cr,
		const Eigen::VectorXd& x, const Eigen::VectorXd& y, const Eigen::VectorXd &V, 
		const Eigen::VectorXd& xi, const Eigen::VectorXd& yi, 
		Eigen::VectorXd& Z ) {

  if ( ( x.size() != y.size() ) || 
       ( x.size() != V.size() ) || 
       ( xi.size() != yi.size() ) ) {
    std::cerr << "Error in matrix sizes" << std::endl;
    return 1;
  }

  // Compute the station covariance matrix
  // With an extra row/column for Langrange parameter in Kriging
  Eigen::MatrixXd stCovMat = Eigen::MatrixXd::Ones(x.size()+1,x.size()+1); 

  std::cout << "Allocating station covariance matrix, rows=" 
	    << stCovMat.rows() << ", cols = " << stCovMat.cols() 
	    << std::endl;

  cr->corrmat( x, y, x, y, stCovMat );
  stCovMat(x.size(),x.size()) = 0.; // set bottom right element to 0.

  //throw "STOP HERE";

  std::ofstream ofs( "st_covmat.out" );
  ofs << stCovMat;
  ofs.close();


  // covariance matrix between stations and grid locations
  Eigen::MatrixXd grCovMat = Eigen::MatrixXd::Ones(xi.size(),x.size()+1); // gridcells in rows
  std::cout << "Allocating grid covariance matrix, rows=" 
	    << grCovMat.rows() << ", cols = " << grCovMat.cols() 
	    << std::endl;
  cr->corrmat( x, y, xi, yi, grCovMat );

  std::ofstream ofs2( "gr_covmat.out" );
  ofs2 << grCovMat;
  ofs2.close();

  std::cout << "stCovMat [C]            : rows = " 
	    << stCovMat.rows() << ", cols = " << stCovMat.cols() << std::endl; 
  std::cout << "grCovMat [D1 D2 D3 ...] : rows = " 
	    << grCovMat.rows() << ", cols = " << grCovMat.cols() << std::endl; 

  //Eigen::MatrixXd w = stCovMat.ldlt().solve(grCovMat.transpose()).transpose();

  
  // Eigen expression to solve for the whole array of grid values
  // some heavy vectorisation going on here....
  Z = (stCovMat.ldlt().solve(grCovMat.transpose()).transpose()).block(0,0,xi.size(),x.size())*V;

  return 0;
}


int main( int argc, char *argv[] ) {
  
  Eigen::initParallel();

  if ( argc != 3 ) {
    std::cerr << "Usage" << std::endl << "  ipol_krige_test <stations.txt> <grid.txt>" << std::endl;
    return 1;
  }
  network net( argv[1] );
  grid    gr( argv[2] );

  // std::cout << net << std::endl;

  net.find( "42R802" )->setState( false );
  net.find( "45R512" )->setState( false );

  station *s = net.find( "42R802" );
  std::cout << net.find( "42R802" )->getX() << std::endl;
  std::cout << *s << std::endl;


  // update the buffers in the network for a specific date/ pollutant...
  // these network & data updating stuff should be handled by the framework !!
  net.updateBuffers();
  Eigen::Map<Eigen::VectorXd> x = Eigen::Map<Eigen::VectorXd>(net.getXCoords(),net.numStations());
  Eigen::Map<Eigen::VectorXd> y = Eigen::Map<Eigen::VectorXd>(net.getYCoords(),net.numStations());
  Eigen::Map<Eigen::VectorXd> v = Eigen::Map<Eigen::VectorXd>(net.getValues(),net.numStations());

  for( std::vector<station*>::const_iterator it = net.list().begin();
       it != net.list().end(); it++ ) {
    std::cout << (*it)->getName() << "\t" << (*it)->getX() << std::endl;
  }

  // now work with Eigen buffers for interpolation
  // read grid

  std::cout << "Grid size : " << gr.size() << std::endl;
  double* gr_x = gr.getX();
  double* gr_y = gr.getY();

  for ( int i=0; i< gr.size(); i++ ) 
    std::cout << "GRID X=" << gr_x[i] << ", GRID Y=" << gr_y[i] << std::endl;

  Eigen::Map<Eigen::VectorXd> xi = Eigen::Map<Eigen::VectorXd>(gr.getX(),gr.size());
  Eigen::Map<Eigen::VectorXd> yi = Eigen::Map<Eigen::VectorXd>(gr.getY(),gr.size());

  SpatialCorrModel cr( "exp" );
  double p[2] = { log(0.95), 200000. }; // value in m, the rest is also in km
  cr.setParams( p, 2 );
  Eigen::VectorXd Z;

  for ( int i=0; i<100; i++ ) {
    ipol_krige( &cr, x, y, v, xi, yi, Z );
  }

  Eigen::MatrixXd st_m(x.size(),3);
  st_m << x, y, v;
  std::ofstream ofs1( "stations.out" );
  ofs1 << std::setprecision(15) << st_m;
  ofs1.close();


  Eigen::MatrixXd gr_m(xi.size(),3);
  gr_m << xi, yi, Z;
  std::ofstream ofs2("grid.out");
  ofs2 << std::setprecision(15) << gr_m;
  ofs2.close();

  std::cout << "Using # threads : " << Eigen::nbThreads() << std::endl;

  return 0;
}
