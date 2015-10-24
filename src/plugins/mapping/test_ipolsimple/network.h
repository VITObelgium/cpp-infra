#ifndef __NETWORK_H
#define __NETWORK_H

#define RIO_STATION_FILE 0
#define XML_STATION_FILE 1

#define MISSING_VALUE -9999

#include <iostream>
#include <string>
#include <vector>

class station;

class network {
public:
  network( const char *fname, int type = RIO_STATION_FILE );
  ~network();
 
  const std::vector<station*>& list(){ return _stlist; }
  friend std::ostream& operator<<(std::ostream& os, const network& net );

  // Station finder
  station* find( std::string st_name );

  int     size( ) { return _stlist.size(); } // return fill size of the network
  int     numStations( ) { return _n; } // return number of available statoins
  double *getXCoords( ) { return _xc; } // get arry of x coords of available statoins
  double *getYCoords( ) { return _yc; }
  int    *getTypes( ) { return _type; }

  double *getValues( ){ return _value; } // return measurements

  // routine should update for a specific date...
  // in the datafile and set the stations accordingly...
  void updateBuffers( void );

private:
  std::vector<station*> _stlist;

  // some buffers here to set 
  double *_xc;
  double *_yc;
  int    *_type;
  double *_value;
  int    _n;
};


#endif /* #ifndef __NETWORK_H */
