#include "infra/process.h"
#include "infra/exception.h"
#include "infra/string.h"

#include <reproc++/run.hpp>

namespace inf {

int run_process(gsl::span<const std::string> command)
{
    reproc::options options;
    options.redirect.parent = true;

    auto [status, ec] = reproc::run(command, options);
    if (ec) {
        throw RuntimeError("Failed to run process ({}).", ec.message());
    }

    return status;
}
}
