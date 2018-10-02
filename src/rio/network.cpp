#include <cstring>
#include <cstdlib>
#include <stdexcept>

#include "parser.hpp"
#include "strfun.hpp"
#include "network.hpp"
#include "xmltools.hpp"

namespace rio
{

network::network()
{
}


network::network( TiXmlElement *el )
{
  if ( ! el ) throw std::runtime_error( "Invalid TiXmlElement pointer in network()..." );

  std::string stationfile;
  try {
    stationfile = rio::xml::getText( el, "filename" );
  } catch ( ... ) {
    throw std::runtime_error( "No <filename> in <stations> tag..." );
  }

  rio::parser::get()->process( stationfile );

  _load_file( stationfile );
}

  
  
network::network( std::string stationfile )
{
  _load_file( stationfile );
}

void network::_load_file( std::string stationfile )
{
  _stations.clear();

  std::cout << " Importing " << stationfile << std::endl;
  FILE *fp = fopen( stationfile.c_str(), "r" );
  if ( ! fp ) throw std::runtime_error( "Unable to open file: " + stationfile );

  char  line[rio::strfun::LINESIZE];
  char *ptok;

  std::vector<double> proxy;
  
  // read header
  fgets( line, sizeof(line), fp );
  // read file
  while ( fgets( line, sizeof(line), fp ) )
  {
    if (rio::strfun::trim(line)[0] == '#' ) continue;
    if ( ! strlen( rio::strfun::trim(line) ) ) continue;

    if ( !( ptok = strtok( line, rio::strfun::SEPCHAR ) ) ) continue;
    int id = atoi( ptok );
    if ( !( ptok = strtok( NULL, rio::strfun::SEPCHAR ) ) ) continue;
    std::string name = ptok;
    if ( !( ptok = strtok( NULL, rio::strfun::SEPCHAR ) ) ) continue;
    double x = atof( ptok );
    if ( !( ptok = strtok( NULL, rio::strfun::SEPCHAR ) ) ) continue;
    double y = atof( ptok );
    if ( !( ptok = strtok( NULL, rio::strfun::SEPCHAR ) ) ) continue;
    double alt = atof( ptok );
    if ( !( ptok = strtok( NULL, rio::strfun::SEPCHAR ) ) ) continue;
    int type = atoi( ptok );
    
    // now read the proxy field...
    proxy.clear();
    while( ptok = strtok( NULL, rio::strfun::SEPCHAR ) ) proxy.push_back( atof( ptok ) );

    // and fill
    station s( name, type, x, y, alt );
    s.setProxy( proxy );

    // add to the network
    _stations.push_back( s );
  }

  fclose(fp);
}

  
network::~network()
{
}


std::ostream& operator<<( std::ostream& out, const network& net )
{

  out << "network:" << std::endl;
  for ( const auto& s : net._stations ) out << s;
  
  return out;
}
  
}
