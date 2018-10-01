//! Polynomial fitter class
/*!
  Eigen-based polynomial fitter, compatibly with the MATLAB polyfit/polyval. 

  The user is required to supply an options structure including the solver mode, which can be : 
  - polyFit<T>::EIGEN_QR_DECOMP : use Householder QR decomposition for solving the vandermonde matrix
  - polyFit<T>::EIGEN_SVD_DECOMP : use singular value decomposition for solving
  - polyFit<T>::EIGEN_NORMAL_EQN : use normal equations for solving

  Example usage, x and y are assumed std::vector<double> with the fit data
  \code{.cpp}
  polyFit<double>::Options opt = { polyFit<double>::EIGEN_QR_DECOMP, 2 };
  polyFit<double> fit( x, y, opt );
  \endcode

  Evaluate for new x data :
  \code{.cpp}
  fit.eval( newx, yhat, yhaterr );
  \endcode

  Note that both in the output parameter array and in the internal vandermonde matrix, the coefficients 
  are returned in ascending order of polynomial power, i.e. : 

  \verbatim
  P(x) = p[0] + p[1]*x + p[2]*x^2 + p[3]*x^3 + ...
  \endverbatim

  /TODO throw some exceptions
  /TODO see how to efficiently move the data back from the Eigen vectors to the std::vectors
*/
#pragma once

#include <exception>
#include <vector>

#include <Eigen/Dense>
#include <Eigen/QR>

namespace infra::numeric {

template <typename T>
class polyFit
{
public:
  

  //! Decomposition type for the vandermonde matrix
  /*! The QR decomposition is what MATLAB uses */
  enum SolverType
  {
    EIGEN_QR_DECOMP,
    EIGEN_SVD_DECOMP,
    EIGEN_NORMAL_EQN
  };

  //! The fit options structure
  typedef struct 
  {
    SolverType    solver;  //!< numerical solver type
    unsigned int  order;   //!< order for the polynomial
  } Options;


public:

  //! polyFit constructor
  /*!
    The constructor acutally already performs the fit and stores the QR decomposition of the Vandermonde
    matrix as well as it's inverse for later efficient evaluation of the fitted polynomial and it's error
    via the eval method.
    /param x const reference to a vector of x-values
    /param y const reference to a vector of y-values
    /param opt the options structure
   */
  polyFit( const std::vector<T>& x, const std::vector<T>& y, polyFit::Options& opt )
    : _opts( opt )
  {

    // some checks
    assert(x.size() != 0 );
    assert(y.size() != 0 );
    assert(x.size() == y.size());

    // map to eigen fields
    vector_t yv = Eigen::VectorXd::Map(&y.front(), y.size());
    vector_t xv = Eigen::VectorXd::Map(&x.front(), x.size());

    // build vandermode matrix
    // note that the highest power is first
    _V.resize( x.size(), _opts.order + 1);
    for (size_t i = 0; i < x.size(); i++)
      for (size_t j = 0; j < _opts.order+1; j++ )
	_V(i, j) = std::pow(x.at(i), j); // pow(double,int) overloaded in C++ should be efficient enough

    //      for (size_t j = _opts.order+1; j --> 0; )
    //	_V(i, _opts.order-j) = std::pow(x.at(i), j); // pow(double,int) overloaded in C++ should be efficient enough
    
    //degrees of freedom
    _df = y.size() - ( _opts.order + 1 );
    if ( _df < 0 ) _df = 0;
    assert( _df > 0 );

    // calculate QR decomposition and get the upper triangle part (R), store the inverse of that
    // in the object, will be used for later calculation of the uncertainty in eval();
    Eigen::HouseholderQR<matrix_t> qr;
    qr.compute( _V );
    _R    = qr.matrixQR().topRows(_opts.order+1).template triangularView<Eigen::Upper>();
    _Rinv = _R.inverse();
    
    // solve for linear least sqaures fit
    switch ( _opts.solver )
    {
    case EIGEN_QR_DECOMP: // QR decomposition,  good compromise
      _p = qr.solve(yv);
      break;
    case EIGEN_NORMAL_EQN: // using normal equations, fastest but least accurate
      _p = ( _V.transpose() * _V).ldlt().solve(_V.transpose() * yv);
      break;
    case EIGEN_SVD_DECOMP: // case, default : SVD; most accurate, but slowest
      _p = _V.jacobiSvd( Eigen::ComputeThinU | Eigen::ComputeThinV ).solve(yv);
      break;
    default:
      throw std::exception();
      break;
    }

    // compute residues etc...
    _residus = yv - _V*_p;
    _normr   = _residus.norm();
  }

  virtual ~polyFit()
  {}

  //! evaluate for a vector of x values, without the errors
  /*!
    Evaluates the fitted polynomial for a vector of x values. The y vector is cleared and 
    accordingly. This one does not compute the fit errors
    /param x constant reference to the vector with x values to evaluate
    /param yhat reference to the vector with y values to fill with the result
  */
  void eval( const std::vector<T>& x, std::vector<T>& yhat ) const
  {
    matrix_t V;   

    // map to eigen fields
    vector_t xv   = Eigen::VectorXd::Map(&x.front(), x.size());
    vector_t yv;

    // build vandermode matrix
    V.resize( x.size(), _opts.order + 1);
    for (size_t i = 0; i < x.size(); i++)
      for (size_t j = 0; j < _opts.order+1; j++ )
	V(i,j) = std::pow(x.at(i), j); // pow(double,int) overloaded in C++ should be efficient enough

    //MATLAB uses this form of the Vandermonde matrix... 
    //for (size_t j = _opts.order+1; j --> 0  ;)
    //V(i, _opts.order-j) = std::pow(x.at(i), j); // pow(double,int) overloaded in C++ should be efficient enough

    // Compute fit results
    yv = V * _p;

    // copy values back
    yhat.resize( x.size() );
    
    // TODO : how to directly fill the memory of the yhat/yhaterr , without having to copy
    //        the Eigen::Map doesn't seem to return ????? 
    for ( size_t i = 0; i < x.size(); i ++ )
    {
      yhat[i] = yv[i];
    }
    
    return;
  }

  
  //! evaluate for a vector of x values, including the errors
  /*!
    Evaluates the fitted polynomial for a vector of x values. The y vector is cleared and 
    accordingly. This version computes the fit errors
    /param x constant reference to the vector with x values to evaluate
    /param yhat reference to the vector with y values to fill with the result
  */
  void eval( const std::vector<T>& x, std::vector<T>& yhat, std::vector<T>& yhaterr ) const
  {
    matrix_t V;   

    // map to eigen fields
    vector_t xv   = Eigen::VectorXd::Map(&x.front(), x.size());
    vector_t yv, yerr;

    // build vandermode matrix
    V.resize( x.size(), _opts.order + 1);
    for (size_t i = 0; i < x.size(); i++)
      for (size_t j = 0; j < _opts.order+1; j++ )
	V(i,j) = std::pow(x.at(i), j); // pow(double,int) overloaded in C++ should be efficient enough

    //MATLAB uses this form of the Vandermonde matrix... 
    //for (size_t j = _opts.order+1; j --> 0  ;)
    //V(i, _opts.order-j) = std::pow(x.at(i), j); // pow(double,int) overloaded in C++ should be efficient enough

    // Compute fit results
    yv = V * _p;
    
    // compute error
    //E = V/R; --> right matrix division in matlab : E = V*inv(R), so need to convert this in Eigen
    //matrix_t E = _qr.transpose().solve( V.transpose() ).transpose();
    matrix_t E = V*_Rinv;    
    vector_t e = sqrt( 1. + E.array().square().rowwise().sum() );
    // compute vector of errors... 
    yerr = _normr/sqrt(_df)*e;

    // copy values back
    yhat.resize( x.size() );
    yhaterr.resize( x.size() );
    
    // TODO : how to directly fill the memory of the yhat/yhaterr , without having to copy
    //        the Eigen::Map doesn't seem to return ????? 
    for ( size_t i = 0; i < x.size(); i ++ )
    {
      yhat[i] = yv[i];
      yhaterr[i] = yerr[i];
    }
    
    return;
  }

  //! returns the number of parameters in the polynomial
  size_t npar( void ) const { return _p.size(); }

  //! returns the parameter requested, idx corresponds to the power of x, i.e. idx = 0 is the constant
  T par( size_t idx ) const { assert( idx >= 0); assert( idx < _p.size() ); return _p[idx]; }
  
private:
  typedef Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> matrix_t;
  typedef Eigen::Matrix<T, Eigen::Dynamic, 1>              vector_t;

  vector_t _p;     //!< Vector of fitted polynomial coefficients
  matrix_t _V;     //!< Vandermonde matrix for the fit data
  matrix_t _R;     //!< Upper triangular matrix R for the QR decomposition of V
  matrix_t _Rinv;  //!< Inverse of the R matrix from the QR decomposition
  
  Options  _opts;  //!< the fit options structure
  
  vector_t       _residus; //!< the vector of residuals for the fit
  T              _normr;   //!< the norm of the residuals
  unsigned int   _df;      //!< degrees of freedom in the fit
};

} // namespace 
