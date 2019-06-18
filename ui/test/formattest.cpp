#include "uiinfra/format.h"

#include <gtest/gtest.h>

namespace inf::ui::test {

using namespace testing;

TEST(FormatTest, qstring)
{
    QString str("teststring");

    EXPECT_EQ("QString: teststring", fmt::format("QString: {}", str));
}

}
