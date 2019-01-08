#include "gdx/algo/statistics.h"
#include "gdx/denserasterio.h"

#include <fmt/color.h>
#include <fmt/format.h>

#include <algorithm>
#include <limits>

using namespace std::string_literals;

void catMapStats(const std::string& fileName, float max_value, bool summary)
{
    auto raster = gdx::read_dense_raster<float>(fileName);
    auto meta   = raster.metadata();

    auto stats = gdx::statistics(raster, max_value);

    if (summary) {
        fmt::print("{}\n", fileName.data());
        fmt::print(fmt::color::green, "raster geometry\n");
        fmt::print("\trows {}\n", meta.rows);
        fmt::print("\tcols {}\n", meta.cols);
        fmt::print("\txll {}\n", meta.xll);
        fmt::print("\tyll {}\n", meta.yll);
        fmt::print("\tcellsize {}\n", meta.cellSize);
        if (meta.nodata.has_value()) {
            fmt::print("\tnodata {}\n", meta.nodata.value());
        }
    }

    if (max_value > 0) {
        fmt::print(fmt::color::green, "histogram\n");
        if (stats.negativeValues > 0) {
            fmt::print("\t<0\t{}\n", stats.negativeValues);
        }

        uint32_t index = 0;
        for (auto value : stats.histogram) {
            if (value > 0) {
                fmt::print("\t{}\t{}\n", index, value);
            }

            ++index;
        }

        if (stats.countHigh > 0) {
            fmt::print("\t>{}\t{}\n", max_value, stats.countHigh);
        }
    }

    if (summary) {
        fmt::print(fmt::color::green, "raster content\n");
        fmt::print("{} nodata cells\n", stats.noDataValues); // do not put a tab before this, zzm depends on the format of this line as it is
        fmt::print("\t{} zero cells\n", stats.zeroValues);
        fmt::print("\t{} nonzero cells\n", stats.nonZeroValues);
        fmt::print("\tsum {}", stats.sum);

        if (raster.size() > 0) {
            fmt::print(", max {}, min {}\n", stats.highestValue, stats.lowestValue);
        } else {
            fmt::print(", max undefined, min undefined\n");
        }

        if (stats.nonZeroValues > 0) {
            fmt::print("\tAverage excluding zero's: {}\n", stats.sum / stats.nonZeroValues);
        }

        auto zeroNonZero = stats.nonZeroValues + stats.zeroValues;
        if (zeroNonZero > 0) {
            fmt::print("\tAverage including zero's: {}\n", stats.sum / zeroNonZero);
        }

        if (stats.zeroValues) {
            size_t n     = stats.zeroValues;
            double mu    = stats.sum / n;
            double sigma = sqrt(stats.sigmaNonZero / n - mu * mu);
            sigma /= sqrt((double)(n - 1) / (double)(n)); // see page https://en.wikipedia.org/wiki/Variance#Sample_variance
            fmt::print("\tStandard deviation excluding zero's: {}\n", sigma);

            n     = zeroNonZero;
            mu    = stats.sum / n;
            sigma = sqrt(stats.sigmaNonZero / n - mu * mu);
            sigma /= sqrt((double)(n - 1) / (double)(n)); // see page https://en.wikipedia.org/wiki/Variance#Sample_variance
            fmt::print("\tStandard deviation including zero's: {}\n", sigma);
        }
    } else {
        fmt::print("{}: {} nodata, {} zero, {} nonzero\n", fileName.data(), stats.noDataValues, stats.zeroValues, stats.nonZeroValues);
    }
}

int main(int argc, char* argv[])
{
    try {
        if (argc != 2 && argc != 3) {
            fmt::print("usage: {} mapFile [<maxhistogramvalue> | 'summary']\n", argv[0]);
            return EXIT_FAILURE;
        }

        inf::gdal::Registration reg;
        catMapStats(argv[1], argc == 3 ? static_cast<float>(atof(argv[2])) : 0, argc == 3 ? ("summary"s == argv[2]) : false);
        return EXIT_SUCCESS;
    } catch (const std::bad_alloc&) {
        fmt::print(fmt::color::red, "{}: Out of memory\n", argv[0]);
    } catch (const std::exception& e) {
        fmt::print(fmt::color::red, "{}: {}\n", argv[0], e.what());
    }

    return EXIT_FAILURE;
}
