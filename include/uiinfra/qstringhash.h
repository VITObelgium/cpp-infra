#pragma once

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
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
#endif
