#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Engine.h"
#include "ComponentManagerFactory.h"

namespace OPAQ
{
namespace test
{

using namespace testing;
using namespace std::string_literals;

class EngineMock : public IEngine
{
public:
    MOCK_METHOD0(pollutantManager, Config::PollutantManager&());
    MOCK_METHOD0(componentManager, ComponentManager&());
};

class ComponentManagerTest : public Test
{
protected:
    ComponentManagerTest()
    : _cmpMgr(Factory::createComponentManager(_engineMock))
    {
    }

    EngineMock          _engineMock;
    ComponentManager    _cmpMgr;
};

TEST_F(ComponentManagerTest, LoadPlugin)
{
    auto configXml =
        "<config>"
        "    <filename>opaq_fcdb_6UT.h5</filename>"
        "    <basetime_resolution>24</basetime_resolution>"
        "    <fctime_resolution>24</fctime_resolution>"
        "</config>"s;

    TiXmlDocument doc;
    doc.Parse(configXml.c_str(), 0, TIXML_ENCODING_UTF8);
    auto* config = doc.FirstChildElement("config");

    _cmpMgr.loadPlugin("sqlbuffer", "sqlbuffer" PLUGIN_EXT);
    auto& comp = _cmpMgr.createComponent<Component>("sqlbuffer", "sqlbuffer", config);
    EXPECT_EQ(&comp, &_cmpMgr.getComponent<Component>("sqlbuffer"));
}

}
}
