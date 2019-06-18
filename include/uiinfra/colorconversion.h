#pragma once

#include "infra/color.h"

#include <qcolor.h>

namespace inf::ui {

inline QColor toQColor(const inf::Color& color)
{
    return QColor::fromRgba(qRgba(color.r, color.g, color.b, color.a));
}

inline inf::Color fromQColor(const QColor& color)
{
    return inf::Color(uint8_t(color.red()), uint8_t(color.green()), uint8_t(color.blue()), uint8_t(color.alpha()));
}

}
