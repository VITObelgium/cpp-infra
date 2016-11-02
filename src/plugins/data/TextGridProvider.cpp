#include "TextGridProvider.h"
#include "Exceptions.h"
#include "PluginRegistration.h"
#include "tools/XmlTools.h"
#include "tools/StringTools.h"

#include <tinyxml.h>
#include <boost/lexical_cast.hpp>

namespace opaq
{

static std::string s_gridTypePlaceholder = "%grid%";

TextGridProvider::TextGridProvider()
: _logger("TextGridProvider")
{
}

std::string TextGridProvider::name()
{
    return "textgridprovider";
}

void TextGridProvider::configure(TiXmlElement* configuration, const std::string& componentName, IEngine&)
{
    setName(componentName);
    _grid.clear();
    _pattern = XmlTools::getChildValue<std::string>(configuration, "file_pattern");
}

const Grid& TextGridProvider::getGrid(GridType type)
{
    if (_grid[type].cellCount() == 0)
    {
        readFile(type);
    }

    return _grid[type];
}

void TextGridProvider::readFile(GridType type)
{
    std::string filename = _pattern;
    StringTools::replaceAll(filename, s_gridTypePlaceholder, gridTypeToString(type));

    try
    {
        auto& grid = _grid[type];
        auto cellSize = gridTypeToCellSize(type) / 1000.0;

        auto contents = FileTools::readFileContents(filename);
        StringTools::StringSplitter lineSplitter(contents, "\r\n");

        bool first = true;
        for (auto& line : lineSplitter)
        {
            if (first)
            {
                // skip header
                first = false;
                continue;
            }

            // line format: ID XLAMB YLAMB BETA
            StringTools::StringSplitter cellSplitter(line, " \t\r\n\f");
            auto iter = cellSplitter.begin();

            auto id = boost::lexical_cast<long>(*iter++);
            auto x = boost::lexical_cast<double>(*iter++) / 1000.0;
            auto y = boost::lexical_cast<double>(*iter++) / 1000.0;
            //auto beta = boost::lexical_cast<double>(*iter);

            grid.addCell(Cell(id, x, x + cellSize, y, y + cellSize));
        }
    }
    catch (const RunTimeException& e)
    {
        _logger->warn(e.what());
    }
}

OPAQ_REGISTER_STATIC_PLUGIN(TextGridProvider)

}
