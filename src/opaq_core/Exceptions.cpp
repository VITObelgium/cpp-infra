#include "Exceptions.h"

namespace opaq {

EXCEPTION_IMPL(BadConfigurationException, std::runtime_error);
EXCEPTION_IMPL(ComponentAlreadyExistsException, std::runtime_error);
EXCEPTION_IMPL(ComponentNotFoundException, std::runtime_error);
EXCEPTION_IMPL(FailedToLoadPluginException, std::runtime_error);
EXCEPTION_IMPL(PluginNotFoundException, std::runtime_error);

EXCEPTION_IMPL(IOException, std::runtime_error)
EXCEPTION_IMPL(NullPointerException, std::runtime_error);
EXCEPTION_IMPL(ParseException, std::runtime_error)

EXCEPTION_IMPL(NotAvailableException, std::runtime_error);
EXCEPTION_IMPL(ElementNotFoundException, std::runtime_error);

}
