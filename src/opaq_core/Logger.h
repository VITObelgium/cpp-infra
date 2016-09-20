/*
 * Logger.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 *
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <spdlog/spdlog.h>
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

class Logger
{
public:
    Logger(const std::string& name)
    : _logger(Log::createLogger(name))
    {
    }

    spdlog::logger* operator->()
    {
        return _logger.get();
    }

private:
    std::shared_ptr<spdlog::logger> _logger;
};

#endif /* LOGGER_H_ */
