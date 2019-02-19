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
    return inf::Color(color.red(), color.green(), color.blue(), color.alpha());
}

}
