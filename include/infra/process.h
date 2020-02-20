#pragma once

#include <string_view>
#include <string>
#include <gsl/span>

namespace inf {

int run_process(gsl::span<const std::string> command);
}
