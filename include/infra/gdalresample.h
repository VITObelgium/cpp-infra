#pragma once

#include <gdal_version.h>
#include <gdalwarper.h>

namespace inf::gdal {

/* Note: values are selected to be consistent with GDALRIOResampleAlg of gcore/gdal.h */
/*! Warp Resampling Algorithm */
enum class ResampleAlgorithm
{
    NearestNeighbour = GRA_NearestNeighbour, // Nearest neighbour (select on one input pixel)
    Bilinear         = GRA_Bilinear,         // Bilinear (2x2 kernel)
    Cubic            = GRA_Cubic,            // Cubic Convolution Approximation (4x4 kernel)
    CubicSpline      = GRA_CubicSpline,      // Cubic B-Spline Approximation (4x4 kernel)
    Lanczos          = GRA_Lanczos,          // Lanczos windowed sinc interpolation (6x6 kernel)
    Average          = GRA_Average,          // Average (computes the average of all non-NODATA contributing pixels)
    Mode             = GRA_Mode,             // Mode (selects the value which appears most often of all the sampled points)
    Maximum          = GRA_Max,              // Max (selects maximum of all non-NODATA contributing pixels)
    Minimum          = GRA_Min,              // Min (selects minimum of all non-NODATA contributing pixels)
    Median           = GRA_Med,              // Med (selects median of all non-NODATA contributing pixels)
    FirstQuantile    = GRA_Q1,               // Q1 (selects first quartile of all non-NODATA contributing pixels)
    ThirdQuantile    = GRA_Q3,               // Q3 (selects third quartile of all non-NODATA contributing pixels)
#if GDAL_VERSION_NUM >= 3010000
    Sum            = GRA_Sum, // Weighted sum (weighed sum of all non-NODATA contributing pixels)
    RootMeanSquare = GRA_RMS, // Root mean square (weighted root mean square (quadratic mean) of all non-NODATA contributing pixels)
#endif
};

std::string resample_algo_to_string(ResampleAlgorithm algo);

}
