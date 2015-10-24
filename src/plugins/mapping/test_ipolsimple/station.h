#ifndef __STATION_H
#define __STATION_H

#include <iostream>
#include <string>

class station {
public:
  station( std::string name, double x, double y, int type );
  ~station();
  
  friend std::ostream& operator<<(std::ostream& os, const station& s );
  
  std::string getName() { return _name; }
  double      getX() { return _x; }
  double      getY() { return _y; }
  int         getType() { return _type; }
  
  double      getValue() { return _value; }
  
  void        setState( bool tf ) { _isActive = tf; }
  bool        isActive() { return _isActive; }
  
private:
  std::string _name;
  double      _x;
  double      _y;
  int         _type;
  double      _value;
  bool        _isActive;
};


#endif /* __STATION_H */
