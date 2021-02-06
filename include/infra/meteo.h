#pragma once

namespace inf {

inline double celsius_to_kelvin(double celsius)
{
    return 273.15 + celsius;
}

inline double kelvin_to_celsius(double kelvin)
{
    return kelvin - 273.15;
}

}
