#pragma once

#include <vector>
#include <string>
#include <iosfwd>

namespace rio
{

class station
{
public:
  station(); // needed for insert into map apparantly...
  station( std::string name, int type, double x, double y, double alt = 0. );
  virtual ~station();


  // getters
  const std::string& name() const { return _name; }
  double x() const { return _x; }
  double y() const { return _y; }
  double alt() const { return _alt; }
  double type() const { return _type; }
  const std::string& wkt() const { return _wkt; }

  void setProxy( const std::vector<double>& p ) { _proxy = p; }
  const std::vector<double>& proxy() const { return _proxy; } 

  
public:
  friend std::ostream& operator<<( std::ostream& out, const station& s );
  
private:
  std::string _name;
  double      _x;
  double      _y;
  double      _alt;
  int         _type;
  std::string _wkt;

  std::vector<double> _proxy; //! parameters for spatial trend
};


}
