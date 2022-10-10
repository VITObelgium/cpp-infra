#include "infra/gdalresample.h"

namespace inf::gdal {

std::string resample_algo_to_string(ResampleAlgorithm algo)
{
    switch (algo) {
    case ResampleAlgorithm::NearestNeighbour:
        return "NEAREST";
    case ResampleAlgorithm::Bilinear:
        return "BILINEAR";
    case ResampleAlgorithm::Cubic:
        return "CUBIC";
    case ResampleAlgorithm::CubicSpline:
        return "CUBICSPLINE";
    case ResampleAlgorithm::Lanczos:
        return "LANCZOS";
    case ResampleAlgorithm::Average:
        return "AVERAGE";
    case ResampleAlgorithm::Mode:
        return "MODE";
    case ResampleAlgorithm::Maximum:
        return "MAXIMUM";
    case ResampleAlgorithm::Minimum:
        return "MINIMUM";
    case ResampleAlgorithm::Median:
        return "MEDIAN";
    case ResampleAlgorithm::FirstQuantile:
        return "FIRSTQUANTILE";
    case ResampleAlgorithm::ThirdQuantile:
        return "THIRDQUANTILE";
    case ResampleAlgorithm::Sum:
        return "SUM";
    case ResampleAlgorithm::RootMeanSquare:
        return "RMS";
    default:
        return "NONE";
    }
}

}
