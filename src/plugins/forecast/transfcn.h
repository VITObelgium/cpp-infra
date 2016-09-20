#ifndef __TRANSFCN_H
#define __TRANSFCN_H

#include <math.h>

namespace nnet {

  static double purelin( double x ) { return x; }
  static double logsig( double x ) { return 1./(1.+exp(-x)); }
  static double tansig( double x ) { return 2./(1.+exp(-2*x))-1; }
  static double hardlim( double x ) { return ( x < 0 ? 0. : 1. ); }
  static double hardlims( double x ) { return ( x < 0 ? -1. : 1. ); }
  static double poslin( double x ) { return ( x < 0 ? 0 : x ); }
  static double radbas( double x ) { return exp(-x*x); }
  static double satlin( double x ){ return ( x < 0 ? 0 : ( x < 1 ? x : 1 ) ); }
  static double satlins( double x ) { return ( x < -1 ? -1 : ( x < 1 ? x : 1 ) ); }
  static double tribas( double x ) { return ( fabs(x) <= 1 ? 1 - fabs(x) : 0 ); } 

};

#endif /* #ifndef __TRANSFCN_H */
