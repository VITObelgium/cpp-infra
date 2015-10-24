/*
 * Main.cpp
 *
 *  Created on: Jan 16, 2014
 *      Author: vlooys, maiheub
 */
#include <getopt.h>
#include <iostream>
#include <string.h>

#include <DateTime.h>
#include <ConfigurationHandler.h>
#include <Engine.h>
#include <Logger.h>

#include <tools/FileTools.h>
#include <tools/DateTimeTools.h>

#include <config.h>

const std::string OPAQ_VERSION ( VERSION );  // defined in the autoconf config.h

void print_usage( void ) {
  std::cout << "Usage :" << std::endl;
  std::cout << " opaq [options]" << std::endl;
  std::cout << "Available options :" << std::endl;
  std::cout << " --help ................ : this message" << std::endl;
  std::cout << " --cnf <fname> ......... : use this XML config file (def. opaq-config.xml)" << std::endl;
  std::cout << " --logcnf <name> ....... : log4cxx config file (def. Log4cxxConfig.xml)" << std::endl;
  std::cout << " --pol <name> .......... : run for this pollutant/index" << std::endl;
  std::cout << " --basetime <yyyy-mm-dd> : run for this base time" << std::endl;
  std::cout << " --days <number> ....... : run for this many days, starting from base time (def. 1)" << std::endl;
}

void print_welcome( log4cxx::LoggerPtr logger ) {
  logger->info("Welcome to OPAQ - v." + OPAQ_VERSION);
}


int main (int argc, char* argv[]) {

  int c;
  int option_index = 0;

  // define command line options
  struct option long_options[] = {
    { "help",    0, 0, 'h' },
    { "logcnf",  1, 0, 'l' },
    { "cnf",     1, 0, 'c' },
    { "pol",     1, 0, 'p' },
    { "basetime",1, 0, 'b' },
    { "days"    ,1, 0, 'd' },
    { 0,         0, 0,  0} };

  // general variable settable via command line options in opaq
  std::string pol         = "";
  std::string config_file = "opaq-config.xml";
  std::string log_config  = "Log4cxxConfig.xml";
  std::string days        = "1";
  std::string basetime    = "";

  /* -----------------------------------------------------------------------------------
     Parsing command line options
     --------------------------------------------------------------------------------- */
   while( 1 ) {
    c = getopt_long( argc, argv, "o:a:d:p:t:hl", long_options, &option_index );
    if ( c == -1 ) break;
    switch( c ) {
    case 'h':
      print_usage();
      return 0;
      break;
    case 'l': log_config = optarg; break;
    case 'c': config_file = optarg; break;
    case 'p': pol = optarg; break;
    case 'b': basetime = optarg; break;
    case 'd': days = optarg; break;
    default:
      std::cerr << "Error parsing command line options, try --help !" << std::endl;
      return 1;
      break;
    }
   } /* while loop persing command line options */

   // initialize logging framework
   bool loggingViaConfigFile = initLogger(log_config);
   const log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("main");


   // provide some feedback
   print_welcome(logger);

   if (loggingViaConfigFile) {
	   logger->info("Configured logging using " + log_config);
   } else {
	   logger->info("log4cxx config file not found; falling back to console logger");
   }

   logger->info("Using OPAQ config in .... : " + config_file);

  /* -----------------------------------------------------------------------------------
     Starting initialisation
     --------------------------------------------------------------------------------- */

  // Parse configuration
  OPAQ::ConfigurationHandler ch;
  ch.parseConfigurationFile( config_file );
  
  // overwrite a few of the standard run options by the command line options here...
  // 1. pollutant
  if ( pol.size() > 0 ) ch.getOpaqRun()->setPollutantName( pol );
  else pol = ch.getOpaqRun()->getPollutantName();
  logger->info("Requested pollutant ....... : " + pol);
  // 2. base times
  if (basetime.size() > 0) {
	  std::vector<OPAQ::DateTime> * basetimes = &(ch.getOpaqRun()->getBaseTimes());
	  basetimes->clear();
	  OPAQ::DateTime baseTime;
	  try {
		  baseTime = OPAQ::DateTimeTools::parseDate(basetime);
	  } catch (OPAQ::ParseException & e) {
		  logger->error("Failed to parse base time: " + basetime);
		  exit(1);
	  }
	  int dayCount = atoi(days.c_str());
	  for (int i = 0; i < dayCount; i++) {
		  basetimes->push_back(baseTime);
		  baseTime.addDays(1);
	  }
  }
  std::vector<OPAQ::DateTime> * basetimes = &(ch.getOpaqRun()->getBaseTimes());
  logger->info("Requested base times:");
  std::vector<OPAQ::DateTime>::iterator it = basetimes->begin();
  while (it != basetimes->end()) {
	  std::stringstream ss;
	  ss << " " << *it++;
	  logger->info(ss.str());
  }
  
  // validate configuration
  ch.validateConfiguration();

  /* -----------------------------------------------------------------------------------
     Starting Engine...
     --------------------------------------------------------------------------------- */
  OPAQ::Engine engine;
  engine.run( ch.getOpaqRun() );

  // some friendliness
  logger->info ("All done, have a nice day !");
}

