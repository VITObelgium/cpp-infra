#ifndef __EXPCORRELATOR_H
#define __EXPCORRELATOR_H

class ExpCorrelator : public SpatialCorrelator {
public:
  ExpCorrelator( ) : SpatialCorrelator( ) {
    // allocate memory for 2 parameters
    _npar = 2;
    _par  = new double[2];

    // set initial default values
    _par[0] = 1.0;
    _par[1] = 20.0; // tau (take 50 km as default)
  }
  ~ExpCorrelator(){
    delete [] _par;
  }
  
  // functor
  // putting const after the argument list prevents the method
  // to change any of the class members, this is necessary
  // to be able to be used as a functor in Eigen's unaryExpr
  
  // we include already the signature for an anisotropic spatial
  // correlation, but we put the theta to 0 by default so it can
  // be used by a Eigen unaryExpr operatore taking only one argument
  double operator()( double r, double theta = 0 ) const { return p(0)*exp(-r/p(1)); }
};

#endif /* #ifndef __EXPCORRELATOR_H */
