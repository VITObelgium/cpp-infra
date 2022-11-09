#pragma once

#include "infra/color.h"

#include <array>
#include <functional>
#include <vector>

namespace inf {

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

struct ColorMapper
{
    template <typename MapperFunc>
    ColorMapper(MapperFunc&& r, MapperFunc&& g, MapperFunc&& b)
    : red(r), green(g), blue(b)
    {
    }

    std::function<uint8_t(double)> red;
    std::function<uint8_t(double)> green;
    std::function<uint8_t(double)> blue;
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

class ColorMap
{
public:
    ColorMap() = default;
    explicit ColorMap(const ColorDict& cdict, bool reverse = false);
    explicit ColorMap(const std::vector<Color>& clist, bool reverse = false);
    explicit ColorMap(const std::vector<ColorInfo>& clist, bool reverse = false);
    explicit ColorMap(const std::array<Color, 256>& cmap, bool reverse = false);
    explicit ColorMap(const ColorMapper& cmap, bool reverse = false);

    static ColorMap qualitative(const std::vector<Color>& cdict);
    static ColorMap create(std::string_view name);

    // float value in range [0.0-1.0]
    const Color& get_color(float value) const noexcept;
    const Color& get_color(uint8_t value) const noexcept;

    /*! Apply a fadein of the transparancy in the lowest color values
     * /param fadeStop value between 0.0 and 1.0, determines where the colors become opaque
     */
    void apply_opacity_fade_in(float fadeStop);

    /*! Apply a fadeout of the transparancy in the highest color values
     * /param fadeStart value between 0.0 and 1.0, determines where the colors become less opaque
     */
    void apply_opacity_fade_out(float fadeStart);

    constexpr size_t size()
    {
        return _cmap.size();
    }

private:
    uint8_t process_band(float value, const std::vector<ColorDict::Entry>& dict) const noexcept;

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
    static const ColorDict wistia;
    static const ColorDict nipySpectral;

    static const ColorDict gistEarth;
    static const ColorDict gistNcar;
    static const ColorDict gistStern;

    static const ColorMapper rainbow;

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
    static const std::vector<Color> Turbo;

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
