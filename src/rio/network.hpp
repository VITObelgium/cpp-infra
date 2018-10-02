#pragma once

#include <map>
#include <string>
#include <iostream>
#include <memory>

#include <tinyxml.h>

#include "station.hpp"

namespace rio
{

class network
{
public:
  network();
  network( TiXmlElement *el );
  network( std::string stationfile );
  ~network();

  std::shared_ptr<rio::station const> get( const std::string& name ) const
  {
    for ( const rio::station& s : _stations )    
      if ( ! s.name().compare( name ) ) return std::make_shared<rio::station const>(s);        
    return nullptr;    
  }
  
  size_t size( void ) { return _stations.size(); }
  const std::vector<rio::station>& st_list( void ) const { return _stations; }

public:
  friend std::ostream& operator<<( std::ostream& out, const network& net );
  
private:  
  std::vector<rio::station> _stations;

private:
  void _load_file( std::string stationfile );

};


}
