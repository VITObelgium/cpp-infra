#include <iostream>
#include <tinyxml.h>

#include "DetrendedKrigingInterpolator.h"

int main( int argc, char *argv[] ) {
  
  Eigen::initParallel();

  if ( argc != 2 ) {
    std::cout << "Usage :" << std::endl;
    std::cout << "  detr_krige_test config.xml" << std::endl;
    return 1;
  }
  
  const char[] fname_data = "data/pm10_data_rio.txt";
  const char[] fname_grid = "data/clc06d_grid_4x4.txt";
  const char[] fname_stat = "data/pm10_stations_info_GIS_clc06d.txt";

  
  std::cout << "Opening configuration file " << argv[1] << std::endl;
  TiXmlDocument doc( argv[1] );
  if ( ! doc.LoadFile() ) {
    std::cerr << "Error loading XML file " << argv[1] << std::endl;
    return 1;
  }
  
  DetrendedKrigingInterpolator *p;
  try {
    p = new DetrendedKrigingInterpolator( doc.RootElement() );  
  } catch ( char const *s ) {
    std::cout << s << std::endl;
    return 1;
  }
  
  p->ipol( &net, &grid );
  
  return 0;
}
