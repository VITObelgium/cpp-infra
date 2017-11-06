#pragma once

#include <fmt/format.h>
#include <stdexcept>
#include <string>

#define EXCEPTION(NAME, BASE)                             \
    class NAME : public std::exception                    \
    {                                                     \
    public:                                               \
        NAME() = default;                                 \
                                                          \
        template <typename... T>                          \
        NAME(T&&... args)                                 \
        : _message(fmt::format(std::forward<T>(args)...)) \
        {                                                 \
        }                                                 \
                                                          \
        NAME(std::string message)                         \
        : _message(std::move(message))                    \
        {                                                 \
        }                                                 \
                                                          \
        const char* what() const noexcept                 \
        {                                                 \
            return _message.c_str();                      \
        }                                                 \
                                                          \
    private:                                              \
        std::string _message;                             \
    };

namespace infra {

EXCEPTION(RuntimeError, std::runtime_error)
EXCEPTION(InvalidArgument, std::invalid_argument)
EXCEPTION(LicenseError, std::runtime_error)
}
