#pragma once

#include <cassert>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

namespace infra {

bool createDirectoryIfNotExists(const fs::path& path);

/*!Create a full absolute path based on the base path and a relative file path
 * /param base the full path of a file on disk (not a directory)
 * /param file relative filepath that will be combined with the base
 * /return the full absolute path of the relative file argument
 */
fs::path getFullPath(const fs::path& base, const fs::path& file);
}
