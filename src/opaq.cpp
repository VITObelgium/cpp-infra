/*
 * Main.cpp
 *
 *  Created on: Jan 16, 2014
 *      Author: vlooys, maiheub
 */
#include <iostream>
#include <string.h>

#include "ConfigurationHandler.h"
#include "DateTime.h"
#include "Engine.h"
#include "Exceptions.h"

#include "PollutantManager.h"
#include "opaqconfig.h"
#include "plugins/PluginFactory.h"

#include "infra/configdocument.h"
#include "infra/log.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;
using Log    = inf::Log;

static void printVersion()
{
    std::cout << "OPAQ Version: " << OPAQ_VERSION << std::endl;
}

static void printWelcome()
{
    std::cout << "  ______   .______      ___       ______      \n";
    std::cout << " /  __  \\  |   _  \\    /   \\     /  __  \\     \n";
    std::cout << "|  |  |  | |  |_)  |  /  ^  \\   |  |  |  |    \n";
    std::cout << "|  |  |  | |   ___/  /  /_\\  \\  |  |  |  |    \n";
    std::cout << "|  `--'  | |  |     /  _____  \\ |  `--'  '--. \n";
    std::cout << " \\______/  | _|    /__/     \\__\\ \\_____\\_____\\\n\n";
    std::cout << "Welcome to OPAQ - v." << OPAQ_VERSION "\n";
    std::cout << "Created by Bino Maiheu, Stijn van Looy\n";
    std::cout << "Copyright VITO (c) 2015, all rights reserved\n";
    std::cout << "Contact: bino.maiheu@vito.be\n\n";
}

template <typename T>
static T getOptionalArg(const po::variables_map& vm, const char* name, const T& defaultValue)
{
    if (vm.count(name) > 0) {
        return vm[name].as<T>();
    }

    return defaultValue;
}

// have to read in the config file already once to get the log filename,
// have to do this before the logging freamework is initialized, after
// which the configuration handler will parse the config file properly...
std::string readLogName(const std::string& config_file)
{
    try {
        auto doc = inf::ConfigDocument::loadFromFile(config_file);
        return std::string(doc.child("opaq").child("logFile").trimmed_value());
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return "";
}

static const std::string s_emptyString = "";

int main(int argc, char* argv[])
{
    // general variable settable via command line options in opaq
    std::string pol, aggr, basetime, logFile, configFile;
    uint32_t days;

    try {
        /* -----------------------------------------------------------------------------------
           Parsing command line options
           --------------------------------------------------------------------------------- */
        po::options_description desc("Command line options");
        desc.add_options()("help,h", "display help message")("version,v", "show version info")("log,l", po::value<std::string>(), "name for logfile")("cnf,c", po::value<std::string>()->default_value("opaq-config.xml"), "use this XML config file")("pol,p", po::value<std::string>(), "run for this pollutant/index")("aggr,a", po::value<std::string>(), "run for this aggregation time")("basetime,b", po::value<std::string>(), "run for this base time (yyyy-mm-dd)")("days,d", po::value<uint32_t>()->default_value(1), "run for this many days, starting from base time");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help") > 0) {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }

        if (vm.count("version") > 0) {
            printVersion();
            return EXIT_SUCCESS;
        }

        // parse the config file here quickly just to get the log filename if given, cannot do this
        // in the config handler (see remark below..)
        logFile = readLogName(vm["cnf"].as<std::string>());
        if (vm.count("log") > 0) {
            logFile = vm["log"].as<std::string>();
        }

        configFile = vm["cnf"].as<std::string>();
        pol        = getOptionalArg(vm, "pol", s_emptyString);
        aggr       = getOptionalArg(vm, "aggr", s_emptyString);
        basetime   = getOptionalArg(vm, "basetime", s_emptyString);
        days       = vm["days"].as<uint32_t>();
    } catch (const po::error& e) {
        std::cerr << "Invalid command line: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    printWelcome();

    Log::add_console_sink(Log::Colored::On);
    inf::LogRegistration logging("opaq");

    // -- Parse configuration, after init of the log, otherwise we get errors
    opaq::config::PollutantManager pollutantMgr;
    opaq::ConfigurationHandler ch;

    try {
        ch.parseConfigurationFile(configFile, pollutantMgr);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    Log::info("Starting OPAQ run...");
    Log::info("Using OPAQ configuration in .... : {}", configFile);

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
    Log::info("Requested pollutant ....... : {}", pol);
    Log::info("Requested aggregation ..... : {}", opaq::Aggregation::getName(ch.getOpaqRun().getAggregation()));

    // 2. base times
    if (!basetime.empty()) {
        ch.getOpaqRun().clearBaseTimes();

        try {
            auto baseTime = opaq::chrono::from_date_string(basetime);
            for (uint32_t i = 0; i < days; ++i) {
                ch.getOpaqRun().addBaseTime(baseTime);
                baseTime += opaq::chrono::days(1);
            }
        } catch (const opaq::ParseException&) {
            Log::error("Failed to parse base time: {}", basetime);
            exit(EXIT_FAILURE);
        }
    }

#ifndef NDEBUG
    Log::info("Requested base times:");
    for (auto& basetime : ch.getOpaqRun().getBaseTimes())
        Log::info(opaq::chrono::to_string(basetime));
#endif

    opaq::PluginFactory pluginFactory;
    opaq::Engine engine(pollutantMgr, pluginFactory);

    try {
        // validate configuration
        ch.validateConfiguration(pollutantMgr);

        engine.prepareRun(ch.getOpaqRun());
        engine.run(ch.getOpaqRun());
    } catch (const std::exception& e) {
        Log::error("Error during run: {}", e.what());
        return EXIT_FAILURE;
    }

    // some friendliness
    Log::info("All done, have a nice day !");
    return EXIT_SUCCESS;
}
