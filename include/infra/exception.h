#pragma once

#include <fmt/format.h>
#include <stdexcept>
#include <string>

#define EXCEPTION(NAME, BASE)                                     \
    class NAME : public std::exception                            \
    {                                                             \
    public:                                                       \
        NAME() = default;                                         \
                                                                  \
        template <typename... T>                                  \
        NAME(const char* fmtStr, T&&... args)                     \
        : _message(fmt::format(fmtStr, std::forward<T>(args)...)) \
        {                                                         \
        }                                                         \
                                                                  \
        NAME(std::string message)                                 \
        : _message(std::move(message))                            \
        {                                                         \
        }                                                         \
                                                                  \
        NAME(const char* message)                                 \
        : _message(message)                                       \
        {                                                         \
        }                                                         \
                                                                  \
        const char* what() const noexcept                         \
        {                                                         \
            return _message.c_str();                              \
        }                                                         \
                                                                  \
    private:                                                      \
        std::string _message;                                     \
    };

namespace infra {

EXCEPTION(RuntimeError, std::runtime_error)
EXCEPTION(InvalidArgument, std::invalid_argument)
EXCEPTION(LicenseError, std::runtime_error)
EXCEPTION(RangeError, std::out_of_range)
}
