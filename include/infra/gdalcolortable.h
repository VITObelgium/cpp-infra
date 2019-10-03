#pragma once

#include "infra/colormap.h"

#include <gdal_priv.h>

namespace inf::gdal {

class ColorTable
{
public:
    ColorTable(const ColorMap& cm)
    : _ct(GPI_RGB)
    {
        for (int i = 0; i < 255; ++i) {
            auto color = cm.get_color(uint8_t(i));
            GDALColorEntry entry;
            entry.c1 = color.r;
            entry.c2 = color.g;
            entry.c3 = color.b;
            entry.c4 = color.a;
            _ct.SetColorEntry(i, &entry);
        }
    }

    const GDALColorTable* get() const
    {
        return &_ct;
    }

private:
    GDALColorTable _ct;
};

}
