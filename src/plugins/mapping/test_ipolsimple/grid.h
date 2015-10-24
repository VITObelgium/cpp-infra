#ifndef __GRID_H
#define __GRID_H

#include <string>
#include <vector>
#include <map>

class grid {
public:
  grid( const char *fname );
  ~grid();

  int     size( ){ return _x.size(); }
  int     npars(){  return _value.size(); }
  double *getX( ) { return &_x[0]; }
  double *getY( ) { return &_y[0]; }

  double* getParam( std::string name ) {
    // return the pointer to the start of the double array in the corresponding vector
  }

private:
  std::vector<double> _x;
  std::vector<double> _y;
  std::map<std::string, std::vector<double> > _value;
};

#endif /* #ifndef __GRID_H */
