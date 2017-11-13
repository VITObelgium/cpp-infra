#pragma once

#include <cassert>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

namespace infra {

bool createDirectoryIfNotExists(const fs::path& path);
fs::path getFullPath(const fs::path& base, const fs::path& file);
}
