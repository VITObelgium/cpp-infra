#include "gdx/eigeniterationsupport-private.h"
#include "gdx/cpupredicates-private.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <limits>

namespace gdx::test {

MATCHER(FloatEqNan, "Float compare supporting nan")
{
    (void)result_listener;
    float actual   = std::get<0>(arg);
    float expected = std::get<1>(arg);

    return cpu::float_equal_to<float>()(actual, expected);
}
}
