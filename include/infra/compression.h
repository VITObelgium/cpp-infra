#pragma once

#include "infra/exception.h"
#include "infra/span.h"

#include <cassert>
#include <cstdint>
#include <vector>

#ifdef INFRA_COMPRESSION_ZSTD_SUPPORT
#include <zstd.h>
#endif

namespace inf {

enum class CompressionLevel
{
    Minimum,
    Low,
    Regular,
    High,
    Maximum,
};

#ifdef INFRA_COMPRESSION_ZSTD_SUPPORT
namespace zstd {

namespace details {
inline int compression_level_value(CompressionLevel level) noexcept
{
    switch (level) {
    case CompressionLevel::Minimum:
        return 1;
    case CompressionLevel::Low:
        return 5;
    case CompressionLevel::Regular:
        return 10;
    case CompressionLevel::High:
        return 15;
    case CompressionLevel::Maximum:
        return ZSTD_maxCLevel();
    }

    return 1;
}
}

template <typename T>
std::vector<uint8_t> compress(std::span<const T> data, CompressionLevel level)
{
    const auto bufSize = ZSTD_compressBound(data.size() * sizeof(T));

    std::vector<uint8_t> result(bufSize);
    const auto resultSize = ZSTD_compress(result.data(), result.size(), data.data(), data.size() * sizeof(T), details::compression_level_value(level));
    if (ZSTD_isError(resultSize)) {
        throw RuntimeError("Failed to compress data ({})", ZSTD_getErrorName(resultSize));
    }

    result.resize(resultSize);
    return result;
}

template <typename T = uint8_t>
std::vector<T> decompress(std::span<const uint8_t> data)
{
    /* Read the content size from the frame header. For simplicity we require
     * that it is always present. By default, zstd will write the content size
     * in the header when it is known. If you can't guarantee that the frame
     * content size is always written into the header, either use streaming
     * decompression, or ZSTD_decompressBound().
     */
    auto decompressedSize = ZSTD_getFrameContentSize(data.data(), data.size());
    if (decompressedSize == ZSTD_CONTENTSIZE_UNKNOWN) {
        // size not present in header, use the upper bound
        decompressedSize = ZSTD_decompressBound(data.data(), data.size());
    }

    if (decompressedSize == ZSTD_CONTENTSIZE_ERROR) {
        throw RuntimeError("Invalid compressed data");
    }

    assert(decompressedSize % sizeof(T) == 0);
    std::vector<T> result(decompressedSize / sizeof(T));

    const auto resultSize = ZSTD_decompress(result.data(), result.size() * sizeof(T), data.data(), data.size());
    if (ZSTD_isError(resultSize)) {
        throw RuntimeError("Failed to compress data ({})", ZSTD_getErrorName(resultSize));
    }

    assert(resultSize % sizeof(T) == 0);
    result.resize(resultSize / sizeof(T));
    return result;
}

}
#endif
}
