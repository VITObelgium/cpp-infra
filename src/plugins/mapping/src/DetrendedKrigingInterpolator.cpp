#include <iostream>

#include "DetrendedKrigingInterpolator.h"

// List different trend models
#include "PolyTrendModel.h"
#include "MultiLinearTrendModel.h"

// Spatial correlators
#include "ExpCorrelator.h"
#include "ExpWithShortCorrelator.h"

DetrendedKrigingInterpolator::DetrendedKrigingInterpolator( TiXmlElement *cnf ) {
 
  TiXmlElement *pTrend, *pCorr;
  std::string the_type;

  // nullify explicitly
  _spcorModel = 0;
  _trendModel = 0;

  if ( ! cnf ) throw "DetrendedKrigingInterpolator : error in config pointer";
  if ( cnf->ValueStr() != "DetrendedKrigingInterpolator" ) throw "DetrendedKrigingInterpolator: wrong root element";
  
  /*
    Constructing the trend model
  */
  std::cout << "Constructing the trend model..." << std::endl;
  pTrend = cnf->FirstChildElement( "TrendModel" );
  if ( ! pTrend ) {
    std::cout << "No trend model provided, will use ordinary kriging without detrending..." << std::endl;
  } else {
    if ( pTrend->QueryStringAttribute( "type", &the_type ) != TIXML_SUCCESS ) 
      throw "No trend model type attribute given...";

    if ( the_type == "PolyTrendModel" ) {
      std::cout << "Using polynomial trend model..." << std::endl;
      _trendModel = new PolyTrendModel( pTrend ); // will configure based upon xml element tags inside this element
    } else if ( the_type == "MultiLinearTrendModel" ) {
      std::cout << "Using multivariate linear trend model..." << std::endl;
      _trendModel = new MultiLinearTrendModel( ); // no constructor defined yet
    } else
      throw "DetrendedKrigingInterpolator: current implementation does not support this trend model";

  }
  
  /*
    Constructing the spatial correlation model
  */
  std::cout << "Constructing the spatial model..." << std::endl;
  pCorr = cnf->FirstChildElement( "SpatialCorrelator" );
  if ( ! pCorr ) throw "No spatial correlation model provided, this is needed by the kriging !";
  if ( pCorr->QueryStringAttribute( "type", &the_type ) != TIXML_SUCCESS ) 
    throw "No correlation model type attribute given...";

  if ( the_type == "ExpCorrelator" ) {
    std::cout << "Using exponential correlation model" << std::endl; 
    _spcorModel = new ExpCorrelator();
  } else if ( the_type == "ExpWithShortCorrelator" ) {
    std::cout << "Using exponential correlation model with short range trend" << std::endl; 
    _spcorModel = new ExpWithShortCorrelator();
  } else
    throw "DetrendedKrigingInterpolator: current implementation does not support this correlator";
  
}


DetrendedKrigingInterpolator::~DetrendedKrigingInterpolator( ) {
  if ( _spcorModel ) delete _spcorModel;
  if ( _trendModel ) delete _trendModel;
}
