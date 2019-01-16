#pragma once

#include <iosfwd>
#include <vector>
#include <string>

namespace rio
{

class cell
{
public:
  cell( size_t fid, double cx, double cy, double size, double alt = 0. );
  virtual ~cell();

  void setProxy( const std::vector<double>& proxy ) { _proxy = proxy; }
  const std::vector<double>& proxy() const { return _proxy; }

  size_t id() const { return _id; }
  double cx() const { return _cx; }
  double cy() const { return _cy; }
  double alt() const { return _alt; }
  const std::string& wkt() const { return _wkt; }

  double size() const { return _size; }
  double xmin() const { return _cx - .5*_size; }
  double xmax() const { return _cx + .5*_size; }
  double ymin() const { return _cy - .5*_size; }
  double ymax() const { return _cy + .5*_size; }


  
public:
  friend std::ostream& operator<<( std::ostream& out, const cell& c );
  
private:
  size_t _id;
  double _cx;
  double _cy;
  double _size;
  double _alt;

  std::string _wkt;
  std::vector<double> _proxy;
};

} 
