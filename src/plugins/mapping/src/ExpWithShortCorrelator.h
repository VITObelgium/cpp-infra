#ifndef __EXPWITHSHORTCORRELATOR_H
#define __EXPWITHSHORTCORRELATOR_H

#define __MAX__(x,y) ( x > y ? x : y )

class ExpWithShortCorrelator : public SpatialCorrelator {
public:
 ExpWithShortCorrelator( ) : SpatialCorrelator( ) {
    // allocate memory for 5 parameters
    _npar = 4;
    _par  = new double[_npar];
    
    // set initial default values

    // long range correlation
    // A * exp(-r/tau)
    _par[0] = 1.0;  // A
    _par[1] = 20.0; // tau (take 50 km as default)

    // short range correlation
    // ax + b
    _par[2] = -0.001; // a
    _par[3] = 1.;     // b
  }
  
  ~ExpWithShortCorrelator(){
    delete [] _par;
  }
  
  // functor
  // putting const after the argument list prevents the method
  // to change any of the class members, this is necessary
  // to be able to be used as a functor in Eigen's unaryExpr
  
  // we include already the signature for an anisotropic spatial
  // correlation, but we put the theta to 0 by default so it can
  // be used by a Eigen unaryExpr operatore taking only one argument
  double operator()( double r, double theta = 0 ) const { 
    if ( r < 1e-3 ) return 1;
    double corr_l = p(0)*exp(-r/p(1));
    double corr_s = p(2)*r+p(3);
    return __MAX__(0.,__MAX__(corr_l,corr_s)); 
  }
};

#endif /* #ifndef __EXPWITHSHORTCORRELATOR_H */
