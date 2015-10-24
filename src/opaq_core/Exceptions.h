/*
 * Exceptions.h
 *
 *  Created on: Dec 20, 2013
 *      Author: vlooys
 */

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <string>

#define EXCEPTION(NAME)\
class NAME: public std::exception {\
public:\
	NAME() {\
		this->message = #NAME;\
	}\
	NAME(std::string message) {\
		this->message = message;\
	}\
	virtual ~NAME() throw () {};\
	const char * what () const throw () {\
		return message.c_str();\
	}\
private:\
	std::string message;\
};

namespace OPAQ {

EXCEPTION(BadConfigurationException);
EXCEPTION(ComponentAlreadyExistsException);
EXCEPTION(ComponentNotFoundException);
EXCEPTION(FailedToLoadPluginException);
EXCEPTION(PluginNotFoundException);
EXCEPTION(PluginAlreadyLoadedException);

EXCEPTION(IOException)
EXCEPTION(NullPointerException);
EXCEPTION(ParseException)

EXCEPTION(NotConfiguredException)
EXCEPTION(NotAvailableException);
EXCEPTION(ElementNotFoundException);

EXCEPTION(RunTimeException)

} /* namespace opaq */

#endif /* EXCEPTIONS_H_ */


