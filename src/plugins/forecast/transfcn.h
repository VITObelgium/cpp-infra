#ifndef __TRANSFCN_H
#define __TRANSFCN_H

#include <math.h>

namespace nnet {

  inline double purelin( double x ) { return x; }
  inline double logsig( double x ) { return 1./(1.+exp(-x)); }
  inline double tansig( double x ) { return 2./(1.+exp(-2*x))-1; }
  inline double hardlim( double x ) { return ( x < 0 ? 0. : 1. ); }
  inline double hardlims( double x ) { return ( x < 0 ? -1. : 1. ); }
  inline double poslin( double x ) { return ( x < 0 ? 0 : x ); }
  inline double radbas( double x ) { return exp(-x*x); }
  inline double satlin( double x ){ return ( x < 0 ? 0 : ( x < 1 ? x : 1 ) ); }
  inline double satlins( double x ) { return ( x < -1 ? -1 : ( x < 1 ? x : 1 ) ); }
  inline double tribas( double x ) { return ( fabs(x) <= 1 ? 1 - fabs(x) : 0 ); }

};

#endif /* #ifndef __TRANSFCN_H */
