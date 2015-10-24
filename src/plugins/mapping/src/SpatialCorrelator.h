#ifndef __SPATIALCORRELATOR_H
#define __SPATIALCORRELATOR_H

#include <functional>

#include <Eigen/Dense>

class SpatialCorrelator {
public:
  SpatialCorrelator();
  virtual ~SpatialCorrelator();
  
  // pure virtual overloaded () operator, makes this class a functor
  virtual double operator()( double r, double theta = 0 ) const = 0;
  
  /* ================================================================
    WORK HORSE METHODS
    - application to distance matrices
    - or direct calculation of the correlation matrix for some 
    - given coordinate vectors given (*this) object as functor
    ============================================================== */
  
  // method to apply to an Eigen matrix, we assume this matrix has distances
  void apply( Eigen::MatrixXd& rho, const Eigen::MatrixXd& r ); // we can use a unary function on the matrix, if y
  
  // Retrieve a spatial correlation matrix between a set of xs,ys coordinates
  // (as Eigen::Vectors of equal size) and a set of x,y coordinates, equally
  // Eigen::vectors of equal size, the second pair of vectors does not necessarily
  // need to be of the same dimension as the first. provided the xs,ys Vector has
  // ns elements and hte x,y vectors have ng elements, the returned matrix in rho 
  // is of size ng x ns !
  //
  // Also note that if we have a smaller matrix than the one given, we need to resize 
  // the matrix otherwise we leave the dimensions as they are, we only fill the 
  // block(0,0,rows,cols), so this means that the matrix given in rho can already be
  // predifined and when it is larger than the required dimensions, we only fill the top
  // left block of hte matrix and leave the rest untouched. This way we can pre-allocate
  // memory for storing e.g. the lagrange parameter rows in the Kriging matrix, and only 
  // fill the rows/cols relevant to the interpolation at hand. 
  int corrmat( const Eigen::VectorXd &xs, const Eigen::VectorXd &ys,
	       const Eigen::VectorXd &x, const Eigen::VectorXd &y,
	       Eigen::MatrixXd &rho );
  
  // should add a method to derive the spatial correlation matrix for 
  // the air quality network : corrmat( AqNetwork *aq )  


  // get correlation model parameter getters
  double p( int i ) const {
    if ( i < _npar ) return _par[i];
    throw "SpatialCorrelator error : parameter index out of bounds";
    return NAN;
  }
  int np() const { return _npar; }

  // return a pointer to the parameter array
  double* pars( void ) const { return _par; }

  // update the parameters
  void setPars( double *p ) {
    try {
      for(int i=0; i<_npar; i++ )_par[i] = p[i];
    } catch ( ... ) {
      // rethrow the exception and leave handling up to the higher level
      throw; 
    }
    return;
  }
  
protected:
  double *_par; // array of parameters
  int    _npar; // number of parameters
};

#endif /* #ifndef __SPATIALCORRELATOR_H */
