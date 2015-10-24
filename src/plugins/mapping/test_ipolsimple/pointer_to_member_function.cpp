#include <iostream>
#include <string>
#include <functional>
#include <exception>
#include <Eigen/Dense>

/*
  g++ -g -std=c++11 -O3 -I../../depend/Eigen-3.2.0 -o pointer_to_member_function pointer_to_member_function.cpp
*/

using std::string;
 
namespace RIO {

  // abstract base clase for the correlation model functor
  class SpatialCorrelator  {
  public:
    SpatialCorrelator();
    SpatialCorrelator( double *p, int n ) {
      _par = new double[n];
      _npar = n;
      // set values
      for( int i=0; i<n; ++i ) _par[i] = p[i];
    };
    
    virtual ~SpatialCorrelator(){
      std::cout << "Destroying base" << std::endl;
      if ( _par ) delete [] _par;
    }
    
    // overload () operator, makes this class a functor
    virtual double operator()( double r, double theta = 0 ) const = 0;
    
    // method to apply to matrix
    void apply( Eigen::MatrixXd& r ) {
      r = r.unaryExpr( std::ref(*this) );
      return;
    }
    

    // parameter getters
    double p( int i ) const {
      if ( i < _npar ) return _par[i];
      throw "SpatialCorrelator error : parameter index out of bounds";
      return NAN;
    }
    double* pars( void ) const { return _par; }
    void    updatePars( double *p ) {
      try {
	for(int i=0; i<_npar; i++ )_par[i] = p[i];
      } catch ( ... ) {
	throw; // rethrow the exception and leave handling up to the higher level
      }
    }
    
    
  private:
    double *_par; // array of parameters
    int    _npar; // number of parameters
  };
  
  
  class ExpCorrelator : public SpatialCorrelator {
  public:
    ExpCorrelator( double *p ) : SpatialCorrelator( p, 2 ) {
    }
    ~ExpCorrelator(){
      std::cout << "Destroying derived (Exp)" << std::endl;
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
  
  class SphCorrelator : public SpatialCorrelator {
  public:
    SphCorrelator( double *p ) : SpatialCorrelator( p, 2 ) {
    }
    ~SphCorrelator(){
      std::cout << "Destroying derived (Sph)" << std::endl;
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

}; //namespace RIO
 
int main(int argc, char* argv[]){

  // attempt 1: 
  // via pointer to member method...
  //
  //  declare a function pointer to member method
  //  double (CorrModel::*func)( double ) = &CorrModel::fcn;
  //  std::cout << (m1.*func)( 10 ) << std::endl;
  // std::cout << (m2->*func)( 10 ) << std::endl;


  double p[2] = { 0.95, 100. }; // A en tau

  RIO::ExpCorrelator Exp( p );   // stack
  RIO::SpatialCorrelator *Exp2 = new RIO::ExpCorrelator( p ); // via heap pointer
  RIO::SpatialCorrelator *Sph  = new RIO::SphCorrelator( p );

  Eigen::MatrixXd mat1 = Eigen::MatrixXd::Constant(2,2,10);
  Eigen::MatrixXd mat2 = Eigen::MatrixXd::Constant(2,2,10);
  Eigen::MatrixXd mat3 = Eigen::MatrixXd::Constant(2,2,10);


  mat1 = mat1.unaryExpr( std::ref(Exp) );
  std::cout << mat1 << std::endl;

  std::cout << "Application via heap pointer: " << std::endl;
  mat2 = mat2.unaryExpr( std::ref(*Exp2) );
  std::cout << mat2 << std::endl;

  // apply in-situ
  std::cout << "In-situ application : " << std::endl;
  Exp2->apply( mat3 );
  std::cout << mat3 << std::endl;
  p[0] = 0.80;
  p[1] = 50.;
  Exp2->updatePars( p );
  Exp2->apply( mat3 );
  std::cout << mat3 << std::endl;


  std::cout << "Test 2 arguments capability of Exp" << std::endl;
  std::cout << (*Exp2)(10.,5.) << std::endl;

  delete Exp2;
  delete Sph;

  return 0;
}
