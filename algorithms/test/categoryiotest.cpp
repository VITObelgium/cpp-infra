#include "testconfig.h"
#include "gdx/algo/categoryio.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace gdx::test
{

using namespace testing;

static const std::vector<std::vector<double>> s_expectedRows = {
    {1, 10},
    {2, 20},
    {3, 30},
    {4, 40}
};

static const std::vector<std::vector<double>> s_expectedCols = {
    {1, 2, 3, 4},
    {10, 20, 30, 40}
};

TEST(ShapeIOTest, readTabDataAsRows)
{
    auto data = detail::readTabDataRowBased(TEST_DATA_DIR "/testtabfile.tab");
    EXPECT_THAT(s_expectedRows, ContainerEq(data));
}

TEST(ShapeIOTest, readCsvDataAsRows)
{
    auto data = detail::readCsvDataRowBased(TEST_DATA_DIR "/testtabfile.csv");
    EXPECT_THAT(s_expectedRows, ContainerEq(data));
}

TEST(ShapeIOTest, readTabDataAsColumns)
{
    auto data = detail::readTabDataColumnBased(TEST_DATA_DIR "/testtabfile.tab");
    EXPECT_THAT(s_expectedCols, ContainerEq(data));
}

TEST(ShapeIOTest, readCsvDataAsColumns)
{
    auto data = detail::readCsvDataColumnBased(TEST_DATA_DIR "/testtabfile.csv");
    EXPECT_THAT(s_expectedCols, ContainerEq(data));
}

}
