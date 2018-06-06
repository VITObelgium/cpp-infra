#pragma once

#include <fmt/format.h>
#include <stdexcept>
#include <string_view>

#define EXCEPTION(NAME, BASE)                                \
    class NAME : public BASE                                 \
    {                                                        \
    public:                                                  \
        NAME();                                              \
        NAME(std::string_view message);                      \
                                                             \
        template <typename... Args>                          \
        NAME(fmt::CStringRef formatStr, const Args&... args) \
        : BASE(fmt::format(formatStr, args...))              \
        {                                                    \
        }                                                    \
    };

#define EXCEPTION_IMPL(NAME, BASE)       \
    NAME::NAME()                         \
    : BASE("")                           \
    {                                    \
    }                                    \
                                         \
    NAME::NAME(std::string_view message) \
    : BASE(std::string(message))         \
    {                                    \
    }

namespace infra {

EXCEPTION(RuntimeError, std::runtime_error)
EXCEPTION(InvalidArgument, std::invalid_argument)
EXCEPTION(LicenseError, std::runtime_error)
EXCEPTION(RangeError, std::out_of_range)
}
