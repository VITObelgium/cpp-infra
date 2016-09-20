/*
 * Logger.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 *
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <spdlog\spdlog.h>
#include "tools/FileTools.h"

class Log
{
public:
    static void initLogger(const std::string& filename);
    static void initConsoleLogger();
    static void initFileLogger(const std::string& filename);

    static std::shared_ptr<spdlog::logger> getLogger(const std::string& name);
    static std::shared_ptr<spdlog::logger> createLogger(const std::string& filename);

private:
    static std::shared_ptr<spdlog::sinks::sink> _sink;
};

fail; // statically allocated loggers are initialized first and encounter a null sink

#define LOGGER_DEC() \
    static const std::shared_ptr<spdlog::logger> logger;

#define LOGGER_DEF(NAME) \
    const std::shared_ptr<spdlog::logger> NAME::logger = Log::createLogger(#NAME);

#endif /* LOGGER_H_ */
