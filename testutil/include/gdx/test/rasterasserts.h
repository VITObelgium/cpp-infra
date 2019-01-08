#include "gdx/denseraster.h"
#include "gdx/maskedraster.h"
#include "gdx/sparseraster.h"

#include <gtest/gtest.h>

template <typename T>
static std::string printRasterData(const gdx::MaskedRaster<T>& lhs, const gdx::MaskedRaster<T>& rhs)
{
    std::stringstream ss;
    ss << "Data:\n"
       << lhs.eigen_data() << "\n --- \n"
       << rhs.eigen_data()
       << "\nMask:\n"
       << (lhs).mask_data() << "\n --- \n"
       << (rhs).mask_data();

    return ss.str();
}

template <typename T>
static std::string printRasterData(const gdx::DenseRaster<T>& lhs, const gdx::DenseRaster<T>& rhs)
{
    std::stringstream ss;
    ss << "Data:\n"
       << lhs.to_string() << "\n --- \n"
       << rhs.to_string();

    return ss.str();
}

template <typename T>
static std::string printRasterData(const gdx::SparseRaster<T>& lhs, const gdx::SparseRaster<T>& rhs)
{
    std::stringstream ss;
    ss << "Data:\n"
       << lhs.to_string() << "\n --- \n"
       << rhs.to_string();

    return ss.str();
}

#define EXPECT_RASTER_EQ(expected, actual) \
    EXPECT_EQ((expected), (actual)) << printRasterData(expected, actual);

#define EXPECT_RASTER_NEAR(expected, actual) \
    EXPECT_TRUE(expected.tolerant_equal_to((actual))) << printRasterData(expected, actual);

#define EXPECT_RASTER_NE(expected, actual) \
    EXPECT_NE((expected), (actual)) << printRasterData(expected, actual);
