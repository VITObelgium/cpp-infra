#include "TextGridProvider.h"
#include "Exceptions.h"
#include "PluginRegistration.h"
#include "infra/configdocument.h"
#include "tools/StringTools.h"

#include <boost/lexical_cast.hpp>

namespace opaq {

static const char* s_gridTypePlaceholder  = "%grid%";
static const char* s_pollutantPlaceholder = "%pol%";

TextGridProvider::TextGridProvider()
: _logger("TextGridProvider")
{
}

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
    StringTools::replaceAll(filename, s_gridTypePlaceholder, gridTypeToString(type));
    StringTools::replaceAll(filename, s_pollutantPlaceholder, pollutant);

    try {
        auto& grid    = _grid[pollutant][type];
        auto cellSize = gridTypeToCellSize(type);

        auto contents = FileTools::readFileContents(filename);
        StringTools::StringSplitter lineSplitter(contents, "\r\n");

        bool first = true;
        for (auto& line : lineSplitter) {
            if (first) {
                // skip header
                first = false;
                continue;
            }

            // line format: ID XLAMB YLAMB BETA
            StringTools::StringSplitter cellSplitter(line, " \t\r\n\f");
            auto iter = cellSplitter.begin();

            auto id = boost::lexical_cast<long>(*iter++);
            auto x  = boost::lexical_cast<double>(*iter++);
            auto y  = boost::lexical_cast<double>(*iter++);
            //auto beta = boost::lexical_cast<double>(*iter);

            grid.addCell(Cell(id, x, x + cellSize, y, y + cellSize));
        }
    } catch (const RunTimeException& e) {
        _logger->warn(e.what());
    }
}

OPAQ_REGISTER_STATIC_PLUGIN(TextGridProvider)
}
