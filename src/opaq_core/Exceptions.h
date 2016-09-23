/*
 * Exceptions.h
 *
 *  Created on: Dec 20, 2013
 *      Author: vlooys
 */

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <fmt/format.h>
#include <string>

#define EXCEPTION(NAME)                                 \
class NAME : public std::exception                      \
                                                        \
{                                                       \
public:                                                 \
    NAME() = default;                                   \
                                                        \
    template<typename... T>                             \
    NAME(T&&... args)                                   \
    : _message(fmt::format(std::forward<T>(args)...))   \
    {                                                   \
    }                                                   \
                                                        \
    NAME(std::string message)                           \
    : _message(std::move(message))                      \
    {                                                   \
    }                                                   \
                                                        \
    const char* what() const throw()                    \
    {                                                   \
        return _message.c_str();                        \
    }                                                   \
                                                        \
    private : std::string _message;                     \
};

namespace OPAQ
{

EXCEPTION(BadConfigurationException);
EXCEPTION(ComponentAlreadyExistsException);
EXCEPTION(ComponentNotFoundException);
EXCEPTION(FailedToLoadPluginException);
EXCEPTION(PluginNotFoundException);
EXCEPTION(PluginAlreadyLoadedException);

EXCEPTION(IOException)
EXCEPTION(NullPointerException);
EXCEPTION(ParseException)

EXCEPTION(InvalidArgumentsException)
EXCEPTION(NotConfiguredException)
EXCEPTION(NotAvailableException);
EXCEPTION(ElementNotFoundException);

EXCEPTION(RunTimeException)

EXCEPTION(OutOfBoundsException)

} /* namespace opaq */

#endif /* EXCEPTIONS_H_ */
