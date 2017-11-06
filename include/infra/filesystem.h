#pragma once

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

namespace infra {

inline bool createDirectoryIfNotExists(const fs::path& path)
{
    if (fs::exists(path)) {
        return true;
    }

    return fs::create_directories(path);
}
}
