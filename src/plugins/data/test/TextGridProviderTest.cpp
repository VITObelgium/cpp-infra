#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "DateTime.h"
#include "EngineMock.h"
#include "TextGridProvider.h"

#include "tools/FileTools.h"

#include <tinyxml.h>
#include <sstream>

namespace opaq
{
namespace test
{

using namespace date;
using namespace chrono;
using namespace testing;
using namespace std::chrono_literals;
using namespace std::string_literals;

class TextGridProviderTest : public Test
{
protected:
    TextGridProviderTest()
    {
        FileTools::del("pm10_clc06d_grid_4x4.txt");
        FileTools::del("pm10_clc06d_grid_1x1.txt");
        configure();
    }

    void configure()
    {
        auto configXml =
            "<?xml version=\"1.0\"?>"
            "  <config>"
            "    <file_pattern>./%pol%_clc06d_grid_%grid%.txt</file_pattern>"
            "</config>"s;

        TiXmlDocument doc;
        doc.Parse(configXml.c_str(), 0, TIXML_ENCODING_UTF8);
        auto* config = doc.FirstChildElement("config");

        _gridProvider.configure(config, "stationinfoprovider", _engineMock);
    }

    EngineMock _engineMock;
    TextGridProvider _gridProvider;
};

TEST_F(TextGridProviderTest, GetGridCells)
{
    std::stringstream ss;
    ss << "ID	XLAMB	YLAMB	BETA\r\n"
       << "1	24000	174000	0.22\r\n"
       << "2	24000	178000	0.2135\r\n"
       << "3	24000	182000	0.2162\r\n"
       << "4	24000	190000	0.2101\r\n";

    FileTools::writeTextFile("pm10_clc06d_grid_4x4.txt", ss.str());

    auto grid = _gridProvider.getGrid("pm10", GridType::Grid4x4);
    EXPECT_EQ(4u, grid.cellCount());
    EXPECT_EQ(Cell(1, 24.0, 28.0, 174.0, 178.0), grid.cell(0));
    EXPECT_EQ(Cell(2, 24.0, 28.0, 178.0, 182.0), grid.cell(1));
    EXPECT_EQ(Cell(3, 24.0, 28.0, 182.0, 186.0), grid.cell(2));
    EXPECT_EQ(Cell(4, 24.0, 28.0, 190.0, 194.0), grid.cell(3));
}

TEST_F(TextGridProviderTest, GetGridInvalidGridSize)
{
    std::stringstream ss;
    ss << "ID	XLAMB	YLAMB	BETA\r\n"
        << "1	24000	174000	0.22\r\n";

    FileTools::writeTextFile("pm10_clc06d_grid_1x1.txt", ss.str());

    EXPECT_EQ(0, _gridProvider.getGrid("pm10", GridType::Grid4x4).cellCount());
}

TEST_F(TextGridProviderTest, GetGridInvalidPollutant)
{
    EXPECT_EQ(0, _gridProvider.getGrid("pm20", GridType::Grid4x4).cellCount());
}

}
}