#include "infra/exception.h"
#include <string>

namespace infra {

EXCEPTION_IMPL(RuntimeError, std::runtime_error)
EXCEPTION_IMPL(InvalidArgument, std::invalid_argument)
EXCEPTION_IMPL(LicenseError, std::runtime_error)
EXCEPTION_IMPL(RangeError, std::out_of_range)
}
