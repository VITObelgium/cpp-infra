#include "infra/demangle.h"

#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#include <memory>
#endif

namespace inf {

std::string demangle(const char* name)
{
    std::string result;

#ifdef __GNUG__
    int status = -1;
    std::unique_ptr<char, void (*)(void*)> res{
        abi::__cxa_demangle(name, nullptr, nullptr, &status),
        std::free};

    result = (status == 0) ? res.get() : name;
#else
    result = name;
#endif

    return result;
}

}
