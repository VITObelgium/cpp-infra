#include "infra/compression.h"
#include "infra/test/containerasserts.h"

#include <doctest/doctest.h>
#include <numeric>
#include <vector>

namespace inf::test {

using namespace doctest;

TEST_CASE("Compress data with zstd")
{
    std::vector<uint8_t> data(10 * 1024);
    std::iota(data.begin(), data.end(), 0);

    const auto compressedDataHigh = zstd::compress<uint8_t>(data, inf::CompressionLevel::High);
    const auto compressedDataLow  = zstd::compress<uint8_t>(data, inf::CompressionLevel::Low);

    CHECK(compressedDataLow.size() < data.size());
    CHECK(compressedDataHigh.size() <= compressedDataLow.size());

    CHECK_CONTAINER_EQ(data, zstd::decompress(compressedDataLow));
    CHECK_CONTAINER_EQ(data, zstd::decompress(compressedDataLow));
}

}
