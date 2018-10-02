#pragma once

#include <string>
#include <map>

namespace rio
{

class parser
{
public:
  static parser* get();
  ~parser();
  
  void process( std::string& s );
  void clear();
  void add_pattern( const std::string key, const std::string value );

  
private:
  parser();
  static parser* _instance;

  std::map<std::string, std::string> _patterns;
};



}
