#pragma once

#include "infra/span.h"

#include <string>
#include <string_view>
#include <vector>

namespace inf {

struct ProcessRunOptions
{
    std::vector<std::pair<std::string, std::string>> environmentVariables;
    std::string workingDirectory;
};

struct ProcessRunResult
{
    int status;
    std::string out;
    std::string err;
};

/*!
    @brief Run a process
    @param command array of executable path with arguments
    @return the status code of the process
    @throw RuntimeError in case of an error launching the process
 */
int run_process(std::span<const std::string> command);

/*!
    @brief Run a process with the option to tweak behavior
    @param command array of executable path with arguments
    @param opts process options
    @return the status code of the process
    @throw RuntimeError in case of an error launching the process
 */
int run_process(std::span<const std::string> command, const ProcessRunOptions& opts);

/*!
    @brief Run a process and capture the program output with the option to tweak behavior
    @param command array of executable path with arguments
    @param opts process options
    @return a ProcessRunResult structure containing the status code and output an error stream results
    @throw RuntimeError in case of an error launching the process
 */
ProcessRunResult run_process_capture_output(std::span<const std::string> command, const ProcessRunOptions& opts);
}
