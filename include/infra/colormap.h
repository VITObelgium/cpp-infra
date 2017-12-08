#pragma once

#include "infra/color.h"

#include <array>
#include <ostream>
#include <vector>

namespace infra {

struct ColorDict
{
    struct Entry
    {
        float x;
        float y0;
        float y1;
    };

    std::vector<Entry> red;
    std::vector<Entry> green;
    std::vector<Entry> blue;
};

struct ColorInfo
{
    ColorInfo(float start_, Color col)
    : start(start_)
    , color(col)
    {
    }

    float start;
    Color color;
};

std::ostream& operator<<(std::ostream& os, const Color& c);

class ColorMap
{
public:
    ColorMap() = default;
    explicit ColorMap(const ColorDict& cdict, bool reverse = false);
    explicit ColorMap(const std::vector<Color>& clist, bool reverse = false);
    explicit ColorMap(const std::vector<ColorInfo>& clist, bool reverse = false);
    explicit ColorMap(const std::array<Color, 256>& cmap, bool reverse = false);

    static ColorMap qualitative(const std::vector<Color>& cdict);
    static ColorMap create(std::string_view name);

    const Color& getColor(float value) const noexcept;
    const Color& getColor(uint8_t value) const noexcept;

private:
    uint8_t processBand(float value, const std::vector<ColorDict::Entry>& dict) const noexcept;

    std::array<Color, 256> _cmap;
};

struct Cmap
{
    static const ColorDict bone;
    static const ColorDict cool;
    static const ColorDict copper;
    static const ColorDict gray;
    static const ColorDict hot;
    static const ColorDict hsv;
    static const ColorDict pink;
    static const ColorDict jet;
    static const ColorDict spring;
    static const ColorDict summer;
    static const ColorDict autumn;
    static const ColorDict winter;
    static const ColorDict spectral;

    static const ColorDict gistEarth;
    static const ColorDict gistNcar;
    static const ColorDict gistStern;

    static const std::vector<ColorInfo> terrain;

    static const std::vector<Color> Blues;
    static const std::vector<Color> BrBG;
    static const std::vector<Color> BuGn;
    static const std::vector<Color> BuPu;
    static const std::vector<Color> GnBu;
    static const std::vector<Color> Greens;
    static const std::vector<Color> Greys;
    static const std::vector<Color> Oranges;
    static const std::vector<Color> OrRd;
    static const std::vector<Color> PiYG;
    static const std::vector<Color> PRGn;
    static const std::vector<Color> PuBu;
    static const std::vector<Color> PuBuGn;
    static const std::vector<Color> PuOr;
    static const std::vector<Color> PuRd;
    static const std::vector<Color> Purples;
    static const std::vector<Color> RdBu;
    static const std::vector<Color> RdGy;
    static const std::vector<Color> RdPu;
    static const std::vector<Color> RdYlBu;
    static const std::vector<Color> RdYlGn;
    static const std::vector<Color> Reds;
    static const std::vector<Color> Spectral;
    static const std::vector<Color> YlGn;
    static const std::vector<Color> YlGnBu;
    static const std::vector<Color> YlOrBr;
    static const std::vector<Color> YlOrRd;

    // Qualitative maps
    static const std::vector<Color> Accent;
    static const std::vector<Color> Dark2;
    static const std::vector<Color> Paired;
    static const std::vector<Color> Pastel1;
    static const std::vector<Color> Pastel2;
    static const std::vector<Color> Set1;
    static const std::vector<Color> Set2;
    static const std::vector<Color> Set3;
    static const std::vector<Color> Tab10;
    static const std::vector<Color> Tab20;
    static const std::vector<Color> Tab20b;
    static const std::vector<Color> Tab20c;
};
}
