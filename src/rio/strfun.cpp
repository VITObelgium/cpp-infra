#include <cstring>
#include <cctype>

#include "strfun.hpp"

namespace rio {
  
namespace strfun {

// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
char *trim( char *str )
{
  char *end;
 
  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}


bool replace(std::string& str, const std::string& from,
             const std::string& to)
{
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

void split( std::vector<std::string>& list, const char *s )
{

  char *ptok;
  char *p = strdup( s );

  list.clear();

  if ( ! ( ptok = strtok( p, SEPCHAR ) ) ) return;
  list.push_back( ptok );
  while ( ptok = strtok( NULL, SEPCHAR ) ) list.push_back( ptok );

  free(p);

  return;
}
  

}

}  
