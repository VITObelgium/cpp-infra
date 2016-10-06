/*
 * Main.cpp
 *
 *  Created on: Jan 16, 2014
 *      Author: vlooys, maiheub
 */
#include <getopt.h>
#include <iostream>
#include <string.h>

#include "ConfigurationHandler.h"
#include "DateTime.h"
#include "Engine.h"
#include "Exceptions.h"
#include "Logger.h"

#include "PollutantManager.h"
#include "tools/DateTimeTools.h"
#include "tools/FileTools.h"

#include "config.h"

void print_usage(void)
{
    std::cout << "Usage :" << std::endl;
    std::cout << " opaq [options]" << std::endl;
    std::cout << "Available options :" << std::endl;
    std::cout << " --help ................ : this message" << std::endl;
    std::cout << " --cnf <fname> ......... : use this XML config file (def. opaq-config.xml)" << std::endl;
    std::cout << " --log <name> .......... : name for logfile (default in <logfile>)" << std::endl;
    std::cout << " --pol <name> .......... : run for this pollutant/index" << std::endl;
    std::cout << " --aggr <aggr> ......... : run for this aggregation time" << std::endl;
    std::cout << " --basetime <yyyy-mm-dd> : run for this base time" << std::endl;
    std::cout << " --days <number> ....... : run for this many days, starting from base time (def. 1)" << std::endl;
    std::cout << std::endl;
}

void printWelcome(void)
{
    std::cout << "  ______   .______      ___       ______      " << std::endl;
    std::cout << " /  __  \\  |   _  \\    /   \\     /  __  \\     " << std::endl;
    std::cout << "|  |  |  | |  |_)  |  /  ^  \\   |  |  |  |    " << std::endl;
    std::cout << "|  |  |  | |   ___/  /  /_\\  \\  |  |  |  |    " << std::endl;
    std::cout << "|  `--'  | |  |     /  _____  \\ |  `--'  '--. " << std::endl;
    std::cout << " \\______/  | _|    /__/     \\__\\ \\_____\\_____\\" << std::endl;
    std::cout << std::endl;
    std::cout << "Welcome to OPAQ - v." << OPAQ_VERSION << std::endl;
    std::cout << "Created by Bino Maiheu, Stijn van Looy" << std::endl;
    std::cout << "Copyright VITO (c) 2015, all rights reserved" << std::endl;
    std::cout << "Contact: bino.maiheu@vito.be" << std::endl;
    std::cout << std::endl;
}

// have to read in the config file already once to the the log filename,
// have to do this before the logging freamework is initialized, after
// which the configuration handler will parse the config file properly...
std::string readLogName(const std::string& config_file)
{
    try
    {
        TiXmlDocument doc(config_file);
        doc.LoadFile(config_file);
        TiXmlElement* rootElement = doc.FirstChildElement("opaq");
        if (rootElement)
        {
            return OPAQ::XmlTools::getText(rootElement, "logfile");
        }
    }
    catch (...)
    {
    }

    return "";
}

int main(int argc, char* argv[])
{
    // define command line options
    struct option long_options[] = {
        {"help", 0, 0, 'h'},
        {"log", 1, 0, 'l'},
        {"cnf", 1, 0, 'c'},
        {"pol", 1, 0, 'p'},
        {"aggr", 1, 0, 'a'},
        {"basetime", 1, 0, 'b'},
        {"days", 1, 0, 'd'},
        {0, 0, 0, 0}};

    // general variable settable via command line options in opaq
    std::string pol, aggr, arg_log, basetime;
    std::string config_file = "opaq-config.xml";
    std::string days        = "1";

    /* -----------------------------------------------------------------------------------
     Parsing command line options
     --------------------------------------------------------------------------------- */
    while (1)
    {
        int option_index = 0;
        int c            = getopt_long(argc, argv, "o:a:d:p:t:hl:", long_options, &option_index);
        if (c == -1) break;
        switch (c)
        {
        case 'h':
            print_usage();
            return EXIT_SUCCESS;
        case 'c': config_file = optarg; break;
        case 'l': arg_log     = optarg; break;
        case 'p': pol         = optarg; break;
        case 'a': aggr        = optarg; break;
        case 'b': basetime    = optarg; break;
        case 'd': days        = optarg; break;
        default:
            std::cerr << "Error parsing command line options, try --help !" << std::endl;
            return EXIT_FAILURE;
        }
    }

    printWelcome();

    // parse the config file here quicly just to get the log filename if given, cannot do this
    // in the config handler (see remark below..)
    std::string log_file = readLogName(config_file);
    if (!arg_log.empty())
    {
        log_file = arg_log;
    }

    Log::initLogger(log_file);
    auto logger = Log::createLogger("main");

    // -- Parse configuration, after init of the log, otherwise we get errors
    OPAQ::Config::PollutantManager pollutantMgr;
    OPAQ::ConfigurationHandler ch;
    
    try
    {
        ch.parseConfigurationFile(config_file, pollutantMgr);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    logger->info("Starting OPAQ run...");
    logger->info("Using OPAQ configuration in .... : {}", config_file);

    /* -----------------------------------------------------------------------------------
     Starting initialization
     --------------------------------------------------------------------------------- */

    // overwrite a few of the standard run options by the command line options here...
    // 1. pollutant

    //TODO do not run for a single pollutant, but for all --> accomodate multi-pollutant models, or alternatively call in a loop
    if (!pol.empty())
        ch.getOpaqRun().setPollutantName(pol, aggr);
    else
        pol = ch.getOpaqRun().getPollutantName();
    logger->info("Requested pollutant ....... : " + pol);
    logger->info("Requested aggregation ..... : " + OPAQ::Aggregation::getName(ch.getOpaqRun().getAggregation()));

    // 2. base times
    if (!basetime.empty())
    {
        auto& basetimes = ch.getOpaqRun().getBaseTimes();
        basetimes.clear();
        
        try
        {
            auto baseTime = OPAQ::DateTimeTools::parseDate(basetime);
            int dayCount  = atoi(days.c_str());
            for (int i = 0; i < dayCount; ++i)
            {
                basetimes.push_back(baseTime);
                baseTime.addDays(1);
            }
        }
        catch (const OPAQ::ParseException&)
        {
            logger->error("Failed to parse base time: {}", basetime);
            exit(EXIT_FAILURE);
        }
    }

#ifdef DEBUG
    logger->info("Requested base times:");
    for (auto& basetime : ch.getOpaqRun().getBaseTimes())
        logger->info(basetime.toString());
#endif

    // validate configuration
    ch.validateConfiguration(pollutantMgr);

    /* -----------------------------------------------------------------------------------
     Starting Engine...
     --------------------------------------------------------------------------------- */
    
    try
    {
        OPAQ::Engine engine(pollutantMgr);
        engine.prepareRun(ch.getOpaqRun());
        engine.run(ch.getOpaqRun());
    }
    catch (const std::exception& e)
    {
        logger->error("Error during run: {}", e.what());
        return EXIT_FAILURE;
    }

    // some friendliness
    logger->info("All done, have a nice day !");
    return EXIT_SUCCESS;
}
