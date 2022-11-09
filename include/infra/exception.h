#pragma once

#include <fmt/format.h>
#include <stdexcept>
#include <string_view>

#define EXCEPTION(NAME, BASE)                                                            \
    class NAME : public BASE                                                             \
    {                                                                                    \
    public:                                                                              \
        NAME();                                                                          \
        NAME(std::string_view message);                                                  \
                                                                                         \
        template <typename... Args>                                                      \
        NAME(const char* formatStr, const Args&... args)                                 \
        : BASE(fmt::format(fmt::runtime(formatStr), std::forward<const Args&>(args)...)) \
        {                                                                                \
        }                                                                                \
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

namespace inf {

EXCEPTION(RuntimeError, std::runtime_error)
EXCEPTION(InvalidArgument, std::invalid_argument)
EXCEPTION(LicenseError, std::runtime_error)
EXCEPTION(NotImplemented, std::runtime_error)
EXCEPTION(RangeError, std::out_of_range)
EXCEPTION(CancelRequested, std::runtime_error)

template <typename EcEnum>
class ErrorCodeException : public RuntimeError
{
public:
    ErrorCodeException(EcEnum ec)
    : _errorCode(ec)
    {
    }

    ErrorCodeException(EcEnum ec, std::string_view message)
    : RuntimeError(message)
    , _errorCode(ec)
    {
    }

    template <typename... Args>
    ErrorCodeException(EcEnum ec, const char* formatStr, const Args&... args)
    : RuntimeError(formatStr, std::forward<const Args&>(args)...)
    , _errorCode(ec)
    {
    }

    EcEnum error_code() const noexcept
    {
        return _errorCode;
    }

private:
    EcEnum _errorCode;
};

}
