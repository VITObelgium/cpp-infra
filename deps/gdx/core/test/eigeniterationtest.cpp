#include "gdx/eigeniterationsupport-private.h"

#include <gsl/span>
#include <gtest/gtest.h>

namespace gdx::test {

TEST(EigenIteration, convertToSpan)
{
    Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> eigenArray;
    eigenArray.resize(10, 20);

    auto span = gsl::make_span(eigenArray.data(), eigenArray.size());
    EXPECT_EQ(eigenArray.size(), span.size());
    EXPECT_EQ(eigenArray.data(), span.data());
}

TEST(EigenIteration, iterate)
{
    Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> eigenArray;
    eigenArray.resize(10, 20);
    eigenArray.fill(5.f);

    for (auto& val : eigenArray) {
        EXPECT_FLOAT_EQ(5.f, val);
    }

    EXPECT_EQ(eigenArray.size(), std::distance(begin(eigenArray), end(eigenArray)));
}
}
