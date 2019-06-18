#pragma once

#include "infra/cast.h"

#include <qstringlist.h>

namespace inf::ui {

template <typename Container>
inline QStringList toQStringList(const Container& cont)
{
    QStringList result;
    result.reserve(inf::truncate<int>(cont.size()));
    std::transform(cont.begin(), cont.end(), std::back_inserter(result), [](const std::string& str) {
        return QString::fromStdString(str);
    });

    return result;
}
}
