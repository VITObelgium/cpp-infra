#include <math.h>
#include <stdio.h>

#include "feedforwardnet.h"
#include "transfcn.h"

#include <iostream>
#include <tinyxml.h>

#include <Eigen/Core>

int main( int argc, char *argv[] ) {

  double fc_val = 0;
  const int n = 7; // number of inputs 
  double inputs[n];
  std::vector<std::pointer_to_unary_function<double,double> > fcns;

  /* ==============================================================
     Test transfer functions
     ============================================================== */
  std::cout << "TRANSFER FUNCTION TESTS" << std::endl;
  FILE *fp = fopen( "test_transfcn.txt", "wt" );
  if ( ! fp ) return 1; 
  fprintf( fp, "x\tpurelin\tlogsig\ttansig\thardlim\t"
	   "hardlims\tposlin\tradbas\tsatlin\tsatlins\ttribas\n" );
  fcns.push_back( std::ptr_fun( nnet::purelin ) );
  fcns.push_back( std::ptr_fun( nnet::logsig ) );
  fcns.push_back( std::ptr_fun( nnet::tansig ) );
  fcns.push_back( std::ptr_fun( nnet::hardlim ) );
  fcns.push_back( std::ptr_fun( nnet::hardlims ) );
  fcns.push_back( std::ptr_fun( nnet::poslin ) );
  fcns.push_back( std::ptr_fun( nnet::radbas ) );
  fcns.push_back( std::ptr_fun( nnet::satlin ) );
  fcns.push_back( std::ptr_fun( nnet::satlins ) );
  fcns.push_back( std::ptr_fun( nnet::tribas ) );

  for ( double x = -5; x < 5; x += 0.01 ) {
    fprintf( fp, "%f", x );
    
    for ( std::vector<std::pointer_to_unary_function<double,double> >::iterator it = fcns.begin() ; 
	  it != fcns.end(); 
	  ++it ) {
      fprintf( fp, "\t%f", (*it)(x) );
    }

    fprintf( fp, "\n" );
  }
  fclose( fp );
  

  /* ==============================================================
     Test feedforward network
     ============================================================== */
  TiXmlDocument doc( "42R801_net2.xml" );
  if ( ! doc.LoadFile() ) {
    std::cerr << "Oops, cannot load xml file..." << std::endl;
    return 1;
  }

  nnet::feedforwardnet *net;
  try {
    net = new nnet::feedforwardnet( doc.RootElement() );
  } catch ( const char *msg ) {
    std::cerr << "Exception caught : " << msg << std::endl;
    return 1;
  } 
  // std::cout << *net << std::endl;

  double sample[7];
  double *output;
  Eigen::VectorXd eigen_sample(7);
  Eigen::VectorXd eigen_output;

  sample[0] = log(1+40.);  // PMMOR
  sample[1] = log(1+36.);  // PMYEST
  sample[2] = log(1+250);  // BLH
  sample[3] = log(1+0.4);  // MCC
  sample[4] = -0.5;        // wdir x
  sample[5] = 1.35;        // wdir y
  sample[6] = 1;           // weekend

  for( int i=0; i < eigen_sample.size(); i++ ) { 
    eigen_sample(i) = sample[i];
  }

  // net->verbose = true;

  net->sim( sample );
  net->getOutput( &output );
  printf( "Output       : %f\n", exp(output[0])-1 );

  net->sim( eigen_sample );
  net->getOutput( eigen_output );
  printf( "Eigen Output : %f\n", exp(eigen_output(0))-1 );

  return 0;
}
