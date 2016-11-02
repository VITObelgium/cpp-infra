#pragma once

#include "../Exceptions.h"

namespace opaq
{

inline void throwOnNullPtr(const void* ptr)
{
    if (ptr == nullptr)
    {
        throw NullPointerException();
    }
}

}
