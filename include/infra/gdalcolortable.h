#pragma once

#include "infra/colormap.h"

#include <gdal_priv.h>

namespace inf::gdal {

class ColorTable
{
public:
    ColorTable()
    : _ct(GPI_RGB)
    {
    }

    ColorTable(const ColorMap& cm)
    : ColorTable()
    {
        for (int i = 0; i < 255; ++i) {
            auto entry = convert_color(cm.get_color(uint8_t(i)));
            _ct.SetColorEntry(i, &entry);
        }
    }

    void set_color_entry(int i, inf::Color color)
    {
        GDALColorEntry entry;
        entry.c1 = color.r;
        entry.c2 = color.g;
        entry.c3 = color.b;
        entry.c4 = color.a;
        _ct.SetColorEntry(i, &entry);
    }

    const GDALColorTable* get() const
    {
        return &_ct;
    }

private:
    static GDALColorEntry convert_color(inf::Color color) noexcept
    {
        GDALColorEntry entry;
        entry.c1 = color.r;
        entry.c2 = color.g;
        entry.c3 = color.b;
        entry.c4 = color.a;
        return entry;
    }

    GDALColorTable _ct;
};

}
