#include "infra/typeinfo.h"
#include "infra/demangle.h"

namespace inf {

std::string type_name(const std::type_info& t)
{
#ifdef _MSC_VER
    // On msvc name contains the decorated name
    return t.name();
#else
    return demangle(t.name());
#endif
}

}
