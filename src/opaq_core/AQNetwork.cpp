#include "AQNetwork.h"

namespace OPAQ {

  AQNetwork::AQNetwork(){
  }

  AQNetwork::~AQNetwork(){
  }


  Station * AQNetwork::findStation( std::string name ) {

    for ( std::vector<Station *>::iterator it = stations.begin(); 
	  it != stations.end(); 
	  ++it ) {
      if ( (*it)->getName() == name ) return (*it);
    }

    return NULL;
  }

}
