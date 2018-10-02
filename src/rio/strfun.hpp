#pragma once

#include <string>
#include <vector>

namespace rio {
  
namespace strfun {

  static constexpr int  LINESIZE  = 1024;
  static constexpr char SEPCHAR[] = " \t;,";

  char *trim( char *str );

  bool replace( std::string& str, const std::string& from, const std::string& to );
  void replaceAll( std::string& str, const std::string& from, const std::string& to );
  void split( std::vector<std::string>& list, const char *s );
  
}

}
