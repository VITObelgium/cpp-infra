#include "gdx/raster.h"

#include "infra/exception.h"
#include <clara.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <regex>

using namespace clara;
using namespace std::string_literals;

int main(int argc, char* argv[])
{
    try {
        struct Cli
        {
            std::string inputRaster, outputRaster;
            bool showHelp = false;
            std::optional<int> epsg;
            std::string type;
        } options;

        auto cli = Help(options.showHelp) |
                   Opt([&](int tol) { options.epsg = tol; }, "number")["-e"]["--epsg"]("Modify epsg") |
                   Opt([&](std::string s) {
                       if (std::regex_match(s, std::regex("^(byte|int|float|double)$"))) {
                           options.type = s;
                           return ParserResult::ok(ParseResultType::Matched);
                       } else {
                           return ParserResult::runtimeError("Type must match : byte|int|float|double");
                       }
                   },
                       "type")["-t"]["--type"]("Change type (byte, int, float, double)") |
                   Arg(options.inputRaster, "input")("input raster") | Arg(options.outputRaster, "output")("output raster");

        if (options.showHelp || argc == 1) {
            fmt::print("{}", cli);
            return EXIT_SUCCESS;
        }

        auto result = cli.parse(Args(argc, argv));
        if (!result) {
            fmt::print(fmt::color::red, "Error in command line: {}", result.errorMessage());
            return EXIT_FAILURE;
        }

        inf::gdal::Registration reg;
        auto raster = gdx::Raster::read(options.inputRaster);
        if (options.epsg.has_value()) {
            raster.set_projection(options.epsg.value());
        }

        auto outputPath = fs::u8path(options.outputRaster);

        if (!options.type.empty()) {
            if (options.type == "float") {
                raster.write(outputPath, typeid(float));
            } else if (options.type == "double") {
                raster.write(outputPath, typeid(double));
            } else if (options.type == "int") {
                raster.write(outputPath, typeid(int32_t));
            } else if (options.type == "byte") {
                raster.write(outputPath, typeid(uint8_t));
            } else {
                throw inf::RuntimeError("Invalid raster type: {}", options.type);
            }
        } else {
            raster.write(outputPath);
        }

        return EXIT_SUCCESS;
    } catch (const std::bad_alloc&) {
        fmt::print(fmt::color::red, "{}: Out of memory\n", argv[0]);
    } catch (const std::exception& e) {
        fmt::print(fmt::color::red, "{}: {}\n", argv[0], e.what());
    }

    return EXIT_FAILURE;
}
