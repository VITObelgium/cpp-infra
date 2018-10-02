#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <tinyxml.h>

#include "cell.hpp"

namespace rio
{

class grid
{
public:
  grid( const std::string& name, TiXmlElement *cnf );
  virtual ~grid();

  const std::vector<cell>& cells() const { return _cells; }

  size_t size() const { return _cells.size(); }

public:
  friend std::ostream& operator<<( std::ostream& out, const grid& g );
  
private:
  std::vector<cell> _cells; 
  double            _cellsize;

  
  void _load_file( std::string filename );
};


}
