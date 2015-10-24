#include <iostream>
#include <Eigen/Dense>

Eigen::MatrixXd Vandermonde( const Eigen::VectorXd& a, int order ) {

  Eigen::MatrixXd V(a.rows(),order+1); 
  V.array().col(0) = Eigen::VectorXd::Ones(a.rows()); // set first column to 1 for bias term
  for( int i=1;i<order+1;++i) V.array().col(i) = pow(a.array(),i);
  
  return V;
}


int main( int argc, char *argv[] ) {

  Eigen::VectorXd a(10);

  for ( int i = 1; i<=10; i++ ) a(i-1) = i; 

  std::cout << a << std::endl;

  Eigen::MatrixXd V = Vandermonde(a,3);
  std::cout << V << std::endl;

  return 0;
}
