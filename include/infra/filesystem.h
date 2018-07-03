#pragma once

#include <iosfwd>
#include <string_view>

#include <cassert>
#ifdef HAVE_FILESYSTEM_H
#include <filesystem>
#if _MSC_VER < 1914
namespace fs = std::experimental::filesystem;
#else
namespace fs = std::filesystem;
#endif
#elif defined HAVE_EXP_FILESYSTEM_H
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#if defined HAVE_FILESYSTEM_H || defined HAVE_EXP_FILESYSTEM_H
#define INFRA_HAS_FILESYSTEM
#endif

namespace infra::file {
#ifdef INFRA_HAS_FILESYSTEM
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

/*!Create a relative path from an absolute path given a root dir
 * /param absPath the absolute path that will be converted to a relative path
 * /param root absolute path of the root directory
 * /return the relative path relative to the root path
 */
fs::path absoluteToRelativePath(const fs::path& absPath, const fs::path& root);

/*!Create an absolute path from a relative path
 * /param relPath the relative path that will be converted to an absolute path
 * /param base base directory for the relative reference
 * /return the absolute path
 */
fs::path relativeToAbsolutePath(const fs::path& relPath, const fs::path& base);

/*!Create a full absolute path based on the base path and a relative file path
 * /param base the full path of a directory on disk (not a file)
 * /param file relative filepath that will be combined with the base
 * /return the full absolute path of the relative file argument
 */
fs::path combinePath(const fs::path& baseDir, const fs::path& file);

std::string readAsText(const fs::path& filename);

#endif

std::string readAsText(const char* filename);
std::string readAsText(const std::string& filename);
std::string readAsText(const std::istream& filestream);
void writeAsText(const std::string& filename, std::string_view contents);
}
