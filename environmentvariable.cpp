#include "infra/environmentvariable.h"

#include <stdlib.h>

namespace inf::env {

void set(std::string_view key, std::string_view value)
{
#ifdef WIN32
    _putenv_s(std::string(key).c_str(), std::string(value).c_str());
#else
    setenv(std::string(key).c_str(), std::string(value).c_str());
#endif
}

}
