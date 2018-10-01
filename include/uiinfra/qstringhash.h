#pragma once

#include <qhash.h>
#include <qstring.h>

namespace std {
template <>
struct hash<QString>
{
    std::size_t operator()(const QString& s) const noexcept
    {
        return qHash(s);
    }
};
}
