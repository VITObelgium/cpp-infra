#pragma once

#include "infra/cast.h"

#include <qstring.h>
#include <string_view>

namespace uiinfra {

inline QString toQString(std::string_view sv)
{
    return QString::fromUtf8(sv.data(), inf::truncate<int>(sv.size()));
}

}
