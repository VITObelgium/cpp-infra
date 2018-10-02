#include "strfun.hpp"
#include "parser.hpp"

namespace rio
{

parser* parser::_instance = NULL;
  
parser* parser::get()
{
  if ( ! _instance )
    _instance = new parser();
  return _instance;
}

parser::parser()
{
}
  
parser::~parser()
{
}
  
void parser::process( std::string& s )
{
  for( auto it : _patterns )
    strfun::replaceAll( s, it.first, it.second );

  return;
}

void parser::clear()
{
  _patterns.clear();
}

  
void parser::add_pattern( const std::string key, const std::string value )
{
  _patterns[key] = value;
}

}
