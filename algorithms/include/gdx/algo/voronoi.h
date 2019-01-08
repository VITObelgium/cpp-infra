#pragma once

#include "gdx/cell.h"
#include "gdx/denseraster.h"
#include "gdx/exception.h"

#include <gsl/span>

namespace gdx {

/*! calculates Voronoi polygons mask for a given list of locations
 *  https://en.wikipedia.org/wiki/Voronoi_diagram
 *  Voronoi polygons(mathematics) are also knows as Thiessen polygons(geophysics)
 *  \param raster raster instance defining the grid to use
 *  \param locations the locations list
 *  All given locations must lie within the given raster
 *  The calculated mask contains 0 in the polygon surrounding the first location,
 *  1 in the polygon surrounding the seconds location, and so on
 *  returns a numpy array containing the calculated Voronoi polygons mask
 */
template <typename RasterType>
RasterType voronoi(const RasterMetadata& meta, gsl::span<const Cell> locations)
{
    using T = typename RasterType::value_type;

    RasterType voronoi(meta, -1);

    if (locations.size() > std::numeric_limits<T>::max()) {
        throw InvalidArgument("More locations provided then possibe for the data type: {}", locations.size());
    }

    // put locations in the grid and check for duplicates
    T locationIndex = 0;
    for (auto& location : locations) {
        auto testIndex = voronoi[location];
        if (testIndex >= 0) {
            throw InvalidArgument("Duplicate cell in locations: {}", location);
        }

        voronoi[location] = locationIndex++;
    }

    // calculate nearest location in each cell
    for (int r = 0; r < voronoi.rows(); ++r) {
        for (int c = 0; c < voronoi.cols(); ++c) {
            if (voronoi(r, c) == -1) {
                double minDist = std::numeric_limits<double>::max();
                T minIndex     = 0;
                T index        = 0;
                for (auto& location : locations) {
                    auto dist = distance(location, Cell(r, c));
                    if (dist < minDist) {
                        minDist       = dist;
                        minIndex      = index;
                        voronoi(r, c) = minIndex;
                    }

                    ++index;
                }
            }
        }
    }

    return voronoi;
}
}
