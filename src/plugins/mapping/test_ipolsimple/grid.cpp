#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <map/trim.h>

#include "grid.h"

#define LINE_SIZE 1024
#define SEPCHAR "\t;, "

grid::grid( const char *fname ) {
  
  FILE *fp = fopen( fname, "r" );
  char line[LINE_SIZE];
  
  if ( ! fp ) throw "Error opening network file...";
  // get header line
  fgets( line, LINE_SIZE, fp );
  while( fgets( line, LINE_SIZE, fp ) ) {
    map::trim(line);
    if ( line[0] == '#' || line[0] == '%' || line[0] == '!' ) continue;
    if ( strlen(line) == 0 ) continue;
    
    // tokenize string & parse different fields...
    strtok( line, SEPCHAR ); // dump ID to dummy
    
    double x = atof( strtok( NULL, SEPCHAR ) );
    double y = atof( strtok( NULL, SEPCHAR ) );
    
    _x.push_back(x);
    _y.push_back(y);
    
  }
  fclose(fp);
  
}

grid::~grid(){
}
