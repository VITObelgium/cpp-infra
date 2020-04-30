#pragma once

#include "infra/span.h"

#include <string>
#include <string_view>

namespace inf {

int run_process(std::span<const std::string> command);
}
