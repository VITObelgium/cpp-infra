/*
  OPAQ simple interpolation plugin
*/
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <map>

#include <Eigen/Core>
#include <Eigen/Dense>

#include "network.h"
#include "station.h"
#include "grid.h"


// --> werk met Ref<> waarschijnlijk om die Map zaken goed te krijgen....
int ipol_idw( const Eigen::VectorXd& x, const Eigen::VectorXd& y, const Eigen::VectorXd &V, 
	      const Eigen::VectorXd& xi, const Eigen::VectorXd& yi, 
	      Eigen::VectorXd& Z, double p = 1 ) {
  
  if ( ( x.size() != y.size() ) || 
       ( x.size() != V.size() ) || 
       ( xi.size() != yi.size() ) ) {
    std::cerr << "Error in matrix sizes" << std::endl;
    return 1;
  }
  
  // compute distance matrix, partly vectorized
  Eigen::MatrixXd w(xi.size(),x.size());
  for( int i=0; i<x.size(); i++ ) {
    w.col(i) = sqrt( pow(xi.array()-x(i),2) + pow(yi.array()-y(i),2) );
  }
  
  // write out distance matrix
  //std::ofstream ofs( "rmat.txt" );
  //ofs << std::setprecision(15) << w;
  //ofs.close();
  
  // compute inverse & power of distance matrix
  w = w.array().pow(-fabs(p));       
  
  // normalize each row to sum over the columns
  // a row corresponds to the weights for a single grid location
  Eigen::VectorXd sum = w.rowwise().sum();
  w.array().colwise() /= sum.array();
  
  // compute interpolations
  Z = w*V;
  
  return 0;
}


int main( int argc, char *argv[] ) {
  
  if ( argc != 3 ) {
    std::cerr << "Usage" << std::endl << "  ipol_idw_test <stations.txt> <grid.txt>" << std::endl;
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

  Eigen::VectorXd Z;
  ipol_idw( x, y, v, xi, yi, Z );
  

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

  return 0;
}
