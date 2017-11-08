#pragma once

#include <cassert>
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

inline fs::path getFullPath(const fs::path& base, const fs::path& file)
{
    assert(base.has_parent_path());
    return fs::canonical(base.parent_path() / file);
}
}
