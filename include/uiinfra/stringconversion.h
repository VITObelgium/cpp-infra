#pragma once

#include "infra/cast.h"
#include "infra/span.h"

#include <qstring.h>
#include <qstringlist.h>
#include <string_view>

namespace inf::ui {

inline QString toQString(std::string_view sv)
{
    return QString::fromUtf8(sv.data(), inf::truncate<int>(sv.size()));
}

inline QStringList stringListFromArray(std::span<const std::string> strings)
{
    QStringList result;
    for (auto s : strings) {
        result.push_back(QString::fromStdString(s));
    }

    return result;
}

}
