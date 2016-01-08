/*
 * Logger.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 *
 *  Depends on log4cxx
 *  	http://logging.apache.org/log4cxx/
 *  	in Ubuntu: sudo aptitude install liblog4cxx10-dev
 */

#ifndef LOGGER_H_
#define LOGGER_H_


#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/patternlayout.h>
#include "tools/FileTools.h"

#define LOGGER_DEC()\
static const log4cxx::LoggerPtr logger;
#define LOGGER_DEF(NAME)\
const log4cxx::LoggerPtr NAME::logger = log4cxx::Logger::getLogger(#NAME);

static void initConsoleLogger( void ) {

	log4cxx::PatternLayout *layout = new log4cxx::PatternLayout();
//	layout->setConversionPattern("%d %-5p (%-25c): %m%n");
	layout->setConversionPattern("%m%n");
	log4cxx::ConsoleAppender * consoleAppender = new log4cxx::ConsoleAppender(layout);
	consoleAppender->setThreshold( log4cxx::Level::getInfo() );
	log4cxx::BasicConfigurator::configure(consoleAppender);

	return;
}

static void initFileLogger( const std::string &fname ) {

	log4cxx::PatternLayout *layout = new log4cxx::PatternLayout();
	layout->setConversionPattern("%d %-5p (%c) %m%n");
	log4cxx::FileAppender *fileAppender = new log4cxx::FileAppender(layout, fname, false ); // do not append, overwrite...
	fileAppender->setThreshold( log4cxx::Level::getInfo() );

	log4cxx::BasicConfigurator::configure(fileAppender);

	return;
}


static bool initLogger( const std::string & logFileName ) {

	if ( logFileName.size() ) {
		initFileLogger( logFileName );
		return true;
	} else {
		initConsoleLogger();
		return false;
	}
}



#endif /* LOGGER_H_ */
