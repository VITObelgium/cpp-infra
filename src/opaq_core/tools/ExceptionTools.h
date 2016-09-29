#pragma once

#include "../Exceptions.h"

namespace OPAQ
{

inline void throwOnNullPtr(void* ptr)
{
    if (ptr == nullptr)
    {
        throw NullPointerException();
    }
}

}
