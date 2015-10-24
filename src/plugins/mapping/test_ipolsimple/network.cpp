#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map/trim.h>

#include "network.h"
#include "station.h"

#define LINE_SIZE 1024
#define SEPCHAR "\t;, "

network::network( const char *fname, int type ) {

    FILE *fp;
    char line[LINE_SIZE];
    
    switch ( type ) {

    case RIO_STATION_FILE:
      fp = fopen( fname, "r" );

      if ( ! fp ) throw "Error opening network file...";
      // get header line
      fgets( line, LINE_SIZE, fp );
      while( fgets( line, LINE_SIZE, fp ) ) {
	map::trim(line);
	if ( line[0] == '#' || line[0] == '%' || line[0] == '!' ) continue;
	if ( strlen(line) == 0 ) continue;
      
	// tokenize string & parse different fields...
	strtok( line, SEPCHAR ); // dump ID to dummy
	std::string st_name = strtok( NULL, SEPCHAR );
	double st_x = atof( strtok( NULL, SEPCHAR ) );
	double st_y = atof( strtok( NULL, SEPCHAR ) );
	int    st_type = atoi( strtok( NULL, SEPCHAR ) );
	// skip beta in RIO station files

	_stlist.push_back( new station( st_name, st_x, st_y, st_type ) );
      }
      fclose(fp);
      break;

    case XML_STATION_FILE:
      throw "XML_STATION_FILE not implemented yet";
      break;

    default:
      throw "Unknown file type for stations";
      break;
    }

    // allocate memoery for buffers
    _xc = new double[_stlist.size()];
    _yc = new double[_stlist.size()];
    _type = new int[_stlist.size()];
    _value = new double[_stlist.size()];

    if ( ! _xc || ! _yc || ! _type || ! _value ) throw "Cannot allocate buffer memory...";

  }

network::~network() {
  // clean up the station list
  for( std::vector<station*>::iterator it = _stlist.begin(); 
       it != _stlist.end(); ++it ) {
    if (*it) delete *it;
  }
  
  // clean buffer memory
  delete [] _xc;
  delete [] _yc;
  delete [] _type;
  delete [] _value;
}


// Station finder
station* network::find( std::string st_name ) {
  for( std::vector<station*>::iterator it = _stlist.begin(); 
       it != _stlist.end(); ++it ) {
    if ( ! st_name.compare( (*it)->getName() ) ) return (*it);
  }
  return NULL;
}


// routine should update for a specific date...
// in the datafile and set the stations accordingly...
void network::updateBuffers( void ) {
  int i = 0;
  for( std::vector<station*>::iterator it = _stlist.begin(); 
       it != _stlist.end(); ++it ) {
    if ( (*it)->isActive() && ( (*it)->getValue() != MISSING_VALUE ) ) {
      _xc[i]    = (*it)->getX();
      _yc[i]    = (*it)->getY();
      _type[i]  = (*it)->getType();
      _value[i] = (*it)->getValue();
      ++i;
    } else {
      std::cout << "Skipping non-active station : " << (*it)->getName() << std::endl;
    }
  }
  // set number
  _n = i;
  return;
}

std::ostream& operator<<(std::ostream &os, const network& net ) {
  os << "<network>" << std::endl;
  for ( std::vector<station*>::const_iterator it = net._stlist.begin() ; 
	it != net._stlist.end(); ++it ) {
    os << *(*it) << std::endl;
  }
  os << "</network>";
  return os;
}
