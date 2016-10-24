#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Engine.h"
#include "ComponentManagerFactory.h"

namespace OPAQ
{
namespace test
{

using namespace testing;

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
    _cmpMgr.loadPlugin("sqlbuffer", "sqlbuffer");
    auto& comp = _cmpMgr.createComponent<Component>("sqlbuffer", "sqlbuffer", nullptr);
    EXPECT_EQ(&comp, &_cmpMgr.getComponent<Component>("sqlbuffer"));
}

}
}
