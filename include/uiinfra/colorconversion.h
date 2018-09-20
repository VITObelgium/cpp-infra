#pragma once

#include "infra/color.h"

#include <qcolor.h>

namespace uiinfra {

QColor toQColor(const inf::Color& color)
{
    return QColor::fromRgba(qRgba(color.r, color.g, color.b, color.a));
}

}
