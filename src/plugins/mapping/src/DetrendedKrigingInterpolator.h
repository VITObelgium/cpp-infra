#ifndef __DETRENDEDKRIGINGINTERPOLATOR_H
#define __DETRENDEDKRIGINGINTERPOLATOR_H

#include <tinyxml.h>

#include "TrendModel.h"
#include "SpatialCorrelator.h"

class DetrendedKrigingInterpolator {
public:
  DetrendedKrigingInterpolator( TiXmlElement *cnf ); // construct the trend model and spatial correlator
  ~DetrendedKrigingInterpolator( );
  
  //void ipol( );

private:
  SpatialCorrelator *_spcorModel;
  TrendModel        *_trendModel;
};


#endif /* #ifndef __DETRENDEDKRIGINGINTERPOLATOR_H */
