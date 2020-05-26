#include "infra/process.h"
#include "infra/exception.h"
#include "infra/string.h"

#include <reproc++/run.hpp>

namespace inf {

int run_process(std::span<const std::string> command)
{
    reproc::options options;
    options.redirect.parent = true;

    auto [status, ec] = reproc::run(command, options);
    if (ec) {
        throw RuntimeError("Failed to run process ({}).", ec.message());
    }

    return status;
}

int run_process(std::span<const std::string> command, const ProcessRunOptions& opts)
{
    reproc::options options;
    options.redirect.parent = true;
    options.environment     = opts.environmentVariables;

    if (!opts.workingDirectory.empty()) {
        options.working_directory = opts.workingDirectory.c_str();
    }

    auto [status, ec] = reproc::run(command, options);
    if (ec) {
        throw RuntimeError("Failed to run process ({}).", ec.message());
    }

    return status;
}

ProcessRunResult run_process_capture_output(std::span<const std::string> command, const ProcessRunOptions& opts)
{
    ProcessRunResult result;

    reproc::options options;
    options.environment = opts.environmentVariables;

    reproc::process process;
    if (auto ec = process.start(command, options); ec) {
        throw RuntimeError("Failed to start process ({}).", ec.message());
    }

    if (!opts.workingDirectory.empty()) {
        options.working_directory = opts.workingDirectory.c_str();
    }

    reproc::sink::string outsink(result.stdout);
    reproc::sink::string errsink(result.stderr);

    if (auto drainec = reproc::drain(process, outsink, errsink); drainec) {
        throw RuntimeError("Failed to read process output ({}).", drainec.message());
    }

    std::error_code ec;
    std::tie(result.status, ec) = process.wait(reproc::infinite);
    if (ec) {
        throw RuntimeError("Failed to run process ({}).", ec.message());
    }

    return result;
}
}
