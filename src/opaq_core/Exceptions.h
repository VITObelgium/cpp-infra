#pragma once

#include "infra/exception.h"

namespace opaq {

EXCEPTION(BadConfigurationException, std::runtime_error);
EXCEPTION(ComponentAlreadyExistsException, std::runtime_error);
EXCEPTION(ComponentNotFoundException, std::runtime_error);
EXCEPTION(FailedToLoadPluginException, std::runtime_error);
EXCEPTION(PluginNotFoundException, std::runtime_error);

EXCEPTION(IOException, std::runtime_error)
EXCEPTION(NullPointerException, std::runtime_error);
EXCEPTION(ParseException, std::runtime_error)

EXCEPTION(NotAvailableException, std::runtime_error);
EXCEPTION(ElementNotFoundException, std::runtime_error);

using RuntimeError    = infra::RuntimeError;
using InvalidArgument = infra::InvalidArgument;

}
