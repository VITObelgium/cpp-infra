#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Engine.h"
#include "PluginFactoryInterface.h"
#include "PollutantManager.h"
#include "infra/xmldocument.h"
#include "opaqconfig.h"
#include "testconfig.h"

namespace opaq {
namespace test {

using namespace inf;
using namespace testing;
using namespace std::string_literals;

class EngineMock : public IEngine
{
public:
    MOCK_METHOD0(pollutantManager, config::PollutantManager&());
    MOCK_METHOD0(componentManager, ComponentManager&());
};

class PluginFactoryMock : public IPluginFactory
{
public:
    MOCK_CONST_METHOD1(createPlugin, std::unique_ptr<Component>(std::string_view));
};

class DummyComponent : public Component
{
public:
    void configure(const inf::XmlNode& /*configuration*/, const std::string& /*componentName*/, IEngine& /*engine*/) override
    {
    }
};

class ComponentManagerTest : public Test
{
protected:
    ComponentManagerTest()
    : _cmpMgr(_engineMock, _pluginFactoryMock)
    {
    }

    PluginFactoryMock _pluginFactoryMock;
    EngineMock _engineMock;
    ComponentManager _cmpMgr;
};

TEST_F(ComponentManagerTest, LoadPlugin)
{
    auto configXml =
        "<config>"
        "    <filename>opaq_fcdb_6UT.h5</filename>"
        "    <basetime_resolution>24</basetime_resolution>"
        "    <fctime_resolution>24</fctime_resolution>"
        "</config>"s;

    auto doc    = XmlDocument::load_from_string(configXml.c_str());
    auto config = doc.child("config");

    EXPECT_CALL(_pluginFactoryMock, createPlugin(std::string_view("dummy"))).WillOnce(Return(ByMove(std::make_unique<DummyComponent>())));

    auto& comp = _cmpMgr.createComponent<Component>("dummy", "dummy", config);
    EXPECT_EQ(&comp, &_cmpMgr.getComponent<Component>("dummy"));
}
}
}
