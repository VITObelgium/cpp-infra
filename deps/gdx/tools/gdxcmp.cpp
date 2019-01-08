#include "gdx/raster.h"
#include "gdx/rasterdiff.h"
#include "gdx/rasteriterator.h"

#include <algorithm>
#include <clara.hpp>
#include <cmath>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iostream>
#include <limits>
#include <optional>

using namespace clara;
using namespace std::string_literals;

bool compareRasters(const gdx::Raster& raster1, const gdx::Raster& raster2, float tolerance, bool ignoreMetadata, bool verbose)
{
    if (!ignoreMetadata) {
        if (raster1.metadata() != raster2.metadata()) {
            fmt::print(fmt::color::red, "Metadata mismatch:\n{}\n{}\n", raster1.metadata().to_string(), raster2.metadata().to_string());
            return false;
        }
    }

    return std::visit([tolerance, verbose](auto&& r1, auto&& r2) {
        auto diff = gdx::diff_rasters(r1, r2, tolerance);

        if (diff.differentCells() == 0) {
            fmt::print(fmt::color::green, "Rasters are equal!\n");
            return true;
        }

        fmt::print(fmt::color::green, "# matches:\t\t{}\n", diff.equal);

        if (diff.dataDifference) {
            fmt::print(fmt::color::red, "# mismatches:\t{}\n", diff.dataDifference);
        }

        if (diff.zeroToNonZero) {
            fmt::print(fmt::color::red, "# zero -> non zero:\t{}\n", diff.zeroToNonZero);
        }

        if (diff.nonZeroToZero) {
            fmt::print(fmt::color::red, "# non zero -> zero:\t{}\n", diff.nonZeroToZero);
        }

        if (diff.zeroToNodata) {
            fmt::print(fmt::color::yellow, "# zero -> nodata:\t{}\n", diff.zeroToNodata);
        }

        if (diff.nonZeroToNodata) {
            fmt::print(fmt::color::yellow, "# non zero -> nodata:\t{}\n", diff.nonZeroToNodata);
        }

        if (diff.nodataToZero) {
            fmt::print(fmt::color::yellow, "# nodata -> zero:\t{}\n", diff.nodataToZero);
        }

        if (diff.nodataToNonZero) {
            fmt::print(fmt::color::yellow, "# nodata -> non zero:\t{}\n", diff.nodataToNonZero);
        }

        if (verbose) {
            using WidestType = decltype(*r1.data() * *r2.data());
            auto pred        = gdx::cpu::float_equal_to<WidestType>(static_cast<WidestType>(tolerance));
            for (int r = 0; r < r1.rows(); r++) {
                for (int c = 0; c < r1.cols(); c++) {
                    if (!r1.is_nodata(r, c) && !r2.is_nodata(r, c) && !pred(static_cast<WidestType>(r1(r, c)), static_cast<WidestType>(r2(r, c)))) {
                        fmt::print(fmt::color::yellow, "value difference at cell ({},{}): {} != {}\n", r, c, r1(r, c), r2(r, c));
                    }
                }
            }
        }

        return false;
    },
        raster1.get(), raster2.get());
}

int main(int argc, char* argv[])
{
    try {
        struct Options
        {
            bool checkMeta = false;
            bool showHelp  = false;
            bool verbose   = false;
            std::optional<float> tolerance;
            std::string expectedRaster, actualRaster;
        } options;

        auto cli = Help(options.showHelp) |
                   Opt(options.checkMeta)["-m"]["--check-meta"]("Check for metadata differences") |
                   Opt(options.verbose)["-v"]["--verbose"]("Verbose output") |
                   Opt([&](float tol) { options.tolerance = tol; }, "number")["-f"]["--floating-point-tolerance"]("Use floating point comparison with given tolerance") |
                   Arg(options.expectedRaster, "expected")("Reference raster") |
                   Arg(options.actualRaster, "actual")("Actual raster");
        ;

        auto result = cli.parse(Args(argc, argv));
        if (!result) {
            fmt::print(fmt::color::red, "Error in command line: {}", result.errorMessage());
            return EXIT_FAILURE;
        }

        if (options.showHelp || argc == 1) {
            fmt::print("{}", cli);
            return EXIT_SUCCESS;
        }

        inf::gdal::Registration reg;
        auto raster1 = gdx::Raster::read(options.expectedRaster);
        auto raster2 = gdx::Raster::read(options.actualRaster);

        if (!compareRasters(raster1, raster2, options.tolerance.value_or(0.f), !options.checkMeta, options.verbose)) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    } catch (const std::bad_alloc&) {
        fmt::print(fmt::color::red, "{}: Out of memory\n", argv[0]);
    } catch (const std::exception& e) {
        fmt::print(fmt::color::red, "{}: {}\n", argv[0], e.what());
    }

    return EXIT_FAILURE;
}
