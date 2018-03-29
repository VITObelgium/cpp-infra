#include "infra/exception.h"
#include <string>

#define EXCEPTION_IMPL(NAME, BASE)       \
    NAME::NAME(std::string_view message) \
    : BASE(std::string(message))         \
    {                                    \
    }

namespace infra {

EXCEPTION_IMPL(RuntimeError, std::runtime_error)
EXCEPTION_IMPL(InvalidArgument, std::invalid_argument)
EXCEPTION_IMPL(LicenseError, std::runtime_error)
EXCEPTION_IMPL(RangeError, std::out_of_range)
}
