#pragma once

#include "gdx/exception.h"

#include <random>

namespace gdx {

template <template <typename> typename RasterType, typename T>
void fillRandom(RasterType<T>& raster, T minValue, T maxValue)
{
    if (minValue > maxValue) {
        throw InvalidArgument("the minimum value must be smaller then the maximum value");
    }

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    const auto size = raster.size();

    if constexpr (RasterType<T>::raster_type_has_nan) {
        std::uniform_real_distribution<T> dis(minValue, maxValue);
        for (int i = 0; i < size; ++i) {
            raster[i] = dis(gen);
        }
    } else if constexpr (std::is_same_v<uint8_t, T>) {
        std::uniform_int_distribution<uint16_t> dis(minValue, maxValue);
        for (int i = 0; i < size; ++i) {
            raster[i] = static_cast<T>(dis(gen));
        }
    } else {
        std::uniform_int_distribution<T> dis(minValue, maxValue);
        for (int i = 0; i < size; ++i) {
            raster[i] = dis(gen);
        }
    }
}

}
