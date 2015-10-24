#include <iostream>

#include "SpatialCorrelator.h"

SpatialCorrelator::SpatialCorrelator( ) {
  _par = 0 ;
}

SpatialCorrelator::~SpatialCorrelator( ) {
}

void SpatialCorrelator::apply( Eigen::MatrixXd& rho, const Eigen::MatrixXd& r ) {
  rho = r.unaryExpr( std::ref(*this) );
}

/*
Eigen::MatrixXd& SpatialCorrelator::apply( const Eigen::MatrixXd& r, const Eigen::Matrix& theta ) {
  r = r.unaryExpr( std::ref(*this) );
  return;
}
*/

int SpatialCorrelator::corrmat( const Eigen::VectorXd &xs, const Eigen::VectorXd &ys,
				const Eigen::VectorXd &x, const Eigen::VectorXd &y,
				Eigen::MatrixXd &rho ) {
  
  int rows = x.size(); // typically number of gridcells
  int cols = xs.size();
  
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
  
  rho.block(0,0,rows,cols) = rho.block(0,0,rows,cols).unaryExpr( std::ref( *this ) );
  return 0;
}
