#include "TextGridProvider.h"
#include "Exceptions.h"
#include "infra/configdocument.h"
#include "infra/filesystem.h"
#include "infra/log.h"
#include "infra/string.h"

#include <boost/lexical_cast.hpp>

namespace opaq {

using namespace infra;

static const LogSource s_logSrc("TextGridProvider");
static const char* s_gridTypePlaceholder  = "%grid%";
static const char* s_pollutantPlaceholder = "%pol%";

std::string TextGridProvider::name()
{
    return "textgridprovider";
}

void TextGridProvider::configure(const infra::ConfigNode& configuration, const std::string& componentName, IEngine&)
{
    setName(componentName);
    _grid.clear();
    _pattern = configuration.child("file_pattern").value();
}

const Grid& TextGridProvider::getGrid(const std::string& pollutant, GridType type)
{
    if (_grid[pollutant][type].cellCount() == 0) {
        readFile(pollutant, type);
    }

    return _grid[pollutant][type];
}

void TextGridProvider::readFile(const std::string& pollutant, GridType type)
{
    std::string filename = _pattern;
    str::replaceInPlace(filename, s_gridTypePlaceholder, gridTypeToString(type));
    str::replaceInPlace(filename, s_pollutantPlaceholder, pollutant);

    try {
        auto& grid    = _grid[pollutant][type];
        auto cellSize = gridTypeToCellSize(type);

        auto contents = file::readAsText(filename);
        str::Splitter lineSplitter(contents, "\r\n", str::StrTokFlags);

        bool first = true;
        for (auto& line : lineSplitter) {
            if (first) {
                // skip header
                first = false;
                continue;
            }

            // line format: ID XLAMB YLAMB BETA
            str::Splitter cellSplitter(line, " \t\r\n\f", str::StrTokFlags);
            auto iter = cellSplitter.begin();

            auto id = boost::lexical_cast<long>(*iter++);
            auto x  = boost::lexical_cast<double>(*iter++);
            auto y  = boost::lexical_cast<double>(*iter++);
            //auto beta = boost::lexical_cast<double>(*iter);

            grid.addCell(Cell(id, x, x + cellSize, y, y + cellSize));
        }
    } catch (const std::exception& e) {
        Log::warn(s_logSrc, e.what());
    }
}
}
