#pragma once

#include <cassert>
#ifdef HAVE_FILESYSTEM_H
#include <filesystem>
#ifdef _MSC_VER
namespace fs = std::experimental::filesystem;
#else
namespace fs = std::filesystem;
#endif
#elif defined HAVE_EXP_FILESYSTEM_H
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace infra {

/*! Create the specified directory and all parent directories if it does
 *  not exist.
 * /param path should specify a directory, not a file path
 * /return true if the directory was created, false otherwise
 */
bool createDirectoryIfNotExists(const fs::path& path);

/*!Create a full absolute path based on the base path and a relative file path
 * /param base the full path of a file on disk (not a directory)
 * /param file relative filepath that will be combined with the base
 * /return the full absolute path of the relative file argument
 */
fs::path combineAbsoluteWithRelativePath(const fs::path& baseFile, const fs::path& file);

/*!Create a full absolute path based on the base path and a relative file path
 * /param base the full path of a directory on disk (not a file)
 * /param file relative filepath that will be combined with the base
 * /return the full absolute path of the relative file argument
 */
fs::path combinePath(const fs::path& baseDir, const fs::path& file);

std::string readTextFile(const fs::path& filename);
}
