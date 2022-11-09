#pragma once

namespace inf {

enum class LegendScaleType
{
    Linear,
    LinearNoOutliers,
    Arithmetic,
    Geometric,
    OverGeometric,
    Harmonic,
    Quantiles,
    StandardisedDescretisation,
    MethodOfBertin,
    NaturalBreaks,
};

}
