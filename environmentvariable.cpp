#include "infra/environmentvariable.h"

#include <stdlib.h>
#include <string>
#include <string_view>

namespace inf::env {

void set(std::string_view key, std::string_view value)
{
#if defined WIN32 || defined __MINGW32__
    _putenv_s(std::string(key).c_str(), std::string(value).c_str());
#else
    setenv(std::string(key).c_str(), std::string(value).c_str(), 1);
#endif
}

}
