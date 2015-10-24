#include <math.h>
#include <stdio.h>

#include "feedforwardnet.h"
#include "transfcn.h"

#include <iostream>
#include <string>
#include <tinyxml.h>
#include <getopt.h>

#include <Eigen/Core>

void print_usage( void ) {

  std::cout << "Usage:" << std::endl;
  std::cout << "  ffsim [options]" << std::endl;
  std::cout << "Available options:" << std::endl;
  std::cout << "  --help ............ : this message" << std::endl;
  std::cout << "  --verbose ......... : verbose output" << std::endl;
  std::cout << "  --input <fname> ... : input samples from file (def: stdin)" << std::endl;
  std::cout << "  --output <fname> .. : ouput samples to file (def: stdout)" << std::endl;
  std::cout << "  --net <fname> ..... : use this network XML (def: net.xml)" << std::endl;
  return;
}

// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
char *trim(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

int main( int argc, char *argv[] ) {

  FILE *inFp, *outFp;
  int option_index = 0;
  struct option long_options[] = {
    { "output",   1, 0, 'o' },
    { "net",      1, 0, 'n' },
    { "input",    1, 0, 'i' },
    { "help",     0, 0, 'h' },
    { "verbose",  0, 0, 'v' },
    { 0,          0, 0,  0  } };

  bool verbose        = false;
  bool need_help      = false;
  std::string input_file   = ""; // default stdin
  std::string output_file  = ""; // default stdout
  std::string network_file = "net.xml";

  // parse command line arguments
  while( 1 ) {
    int c = getopt_long( argc, argv, "o:n:i:hv", long_options, &option_index );
    if ( c == -1 ) break;
    switch( c ) {
    case 'h':
      need_help = true;
      break;
    case 'v':
      verbose = true;
      break;
    case 'o':
      output_file = optarg;
      break;
    case 'i':
      input_file = optarg;
      break;
    case 'n':
      network_file = optarg;
      break;
    default:
      std::cerr << "*** error parsing command line options, see --help !" << std::endl;
      return 1;
      break;
    }
  }

  if ( need_help ) {
    print_usage();
    return 0;
  }


  // Read in the network file
  TiXmlDocument doc( network_file );
  if ( ! doc.LoadFile() ) {
    std::cerr << "*** error, cannot load network xml file..." << std::endl;
    return 1;
  }

  // construct the neural network
  nnet::feedforwardnet *net;
  try {
    net = new nnet::feedforwardnet( doc.RootElement() );
  } catch ( const char *msg ) {
    std::cerr << "*** error, exception caught : " << msg << std::endl;
    return 1;
  } 
  net->verbose = verbose;
  if ( verbose ) {
    std::cout << "Loaded network : " << std::endl;
    std::cout << *net << std::endl;
  }

  // set input/output
  if ( ! input_file.size() ) inFp = stdin;
  else inFp = fopen( input_file.c_str(), "r" );
  if ( ! inFp ) {
    std::cerr << "*** error opening input file/stream" << std::endl; 
    return 1;
  }

  if ( ! output_file.size() ) outFp = stdout;
  else outFp = fopen( output_file.c_str(), "wt" );
  if ( ! outFp ) {
    std::cerr << "*** error opening output file/stream" << std::endl; 
    return 1;
  }


  // size the sample and output vectors
  Eigen::VectorXd sample(net->inputSize());
  Eigen::VectorXd output(net->outputSize());

  char  line[1024];
  char *pch;
  int   idx = 0, ID = 0;
#define SEPCHAR "\t;, "
  while ( fgets( line, 1024, inFp ) != NULL ) {
    trim(line);
    // remove comment
    if ( line[0] == '#' || line[0] == '%' || line[0] == '!' ) continue;
    // remove empty lines
    if ( strlen(line) == 0 ) continue;

    // tokenize string
    ID = atoi(strtok( line, SEPCHAR ));
    if ( verbose )
      std::cout << "Simulating sample " << ID << std::endl;

    pch = strtok( NULL, SEPCHAR );
    idx = 0;
    bool have_error = false;
    while( pch != NULL ) {
      if ( idx == net->inputSize() ) {
	if ( verbose )
	  std::cerr << "*** error, too many features in sample " << ID << std::endl;
	have_error = true;
      }
      sample(idx) = atof(pch);
      idx++;
      pch = strtok( NULL, SEPCHAR);
    }
    if ( idx < net->inputSize() ) {
      if ( verbose )
	std::cerr << "*** error, not enough features in sample " << ID << std::endl;
      have_error = true;
    } 

    if ( have_error ) {
      output << Eigen::VectorXd::Constant(net->outputSize(),-9999);
    } else {
      // run the network
      net->sim( sample );
      net->getOutput( output );
    }

    fprintf( outFp, "%d", ID );
    for ( int i=0; i<net->outputSize(); i++ ) fprintf( outFp, "\t%f", exp(output(i))-1 );
    fprintf( outFp, "\n" );
  }

  // clean up
  fclose( inFp );
  fclose( outFp );

  delete net;

  return 0;
}
