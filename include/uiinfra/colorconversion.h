#pragma once

#include "infra/cast.h"
#include "infra/color.h"
#include "infra/colormap.h"

#include <qbrush.h>
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

inline QGradientStops gradientStopsFromColormap(std::string_view colorMapName)
{
    QGradientStops stops;

    try {
        auto cmap = ColorMap::create(colorMapName);

        stops.reserve(truncate<int>(cmap.size()));
        for (size_t i = 0; i < cmap.size(); ++i) {
            stops.push_back(QGradientStop(i / double(cmap.size() - 1), ui::toQColor(cmap.get_color(truncate<uint8_t>(i)))));
        }
    } catch (const std::exception&) {
        // Unknown color map name...
    }

    return stops;
}

}
