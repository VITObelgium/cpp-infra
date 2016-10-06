#pragma once

#include "../Exceptions.h"

namespace OPAQ
{

inline void throwOnNullPtr(const void* ptr)
{
    if (ptr == nullptr)
    {
        throw NullPointerException();
    }
}

}
