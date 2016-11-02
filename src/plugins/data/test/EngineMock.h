#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Engine.h"

namespace opaq
{
namespace test
{

class EngineMock : public IEngine
{
public:
    MOCK_METHOD0(pollutantManager, config::PollutantManager&());
    MOCK_METHOD0(componentManager, ComponentManager&());
};

}
}
