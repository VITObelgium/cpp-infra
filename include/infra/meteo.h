#pragma once

#include "infra/math.h"

namespace inf {

inline double celsius_to_kelvin(double celsius)
{
    return 273.15 + celsius;
}

inline double kelvin_to_celsius(double kelvin)
{
    return kelvin - 273.15;
}

//https://confluence.ecmwf.int/pages/viewpage.action?pageId=133262398
inline double wind_direction_from_u_v(double u, double v)
{
    return std::fmod(270 - math::rad_to_deg(std::atan2(v, u)), 360.0);
}

inline double wind_speed_from_u_v(double u, double v)
{
    return std::sqrt((u * u) + (v * v));
}

inline std::tuple<double, double> u_v_from_speed_and_direction(double speed, double direction)
{
    const auto radDirection = math::deg_to_rad(direction);

    return {
        speed * (-std::sin(radDirection)),
        speed * (-std::cos(radDirection)),
    };
}

}
