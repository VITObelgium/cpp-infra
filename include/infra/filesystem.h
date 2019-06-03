#pragma once

#include <fmt/core.h>
#include <iosfwd>
#include <string_view>

#include <cassert>
#ifdef HAVE_FILESYSTEM_H
#include <filesystem>
namespace fs = std::filesystem;
#elif defined HAVE_EXP_FILESYSTEM_H
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#elif defined HAVE_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#endif

#if defined HAVE_FILESYSTEM_H || defined HAVE_EXP_FILESYSTEM_H || defined HAVE_BOOST_FILESYSTEM
#define INFRA_HAS_FILESYSTEM
#endif

namespace inf::file {
#ifdef INFRA_HAS_FILESYSTEM
/*! Create the specified directory and all parent directories if it does
 *  not exist.
 * /param path should specify a directory, not a file path
 * /return true if the directory was created, false otherwise
 */
bool create_directory_if_not_exists(const fs::path& path);

/*!Create a full absolute path based on the base path and a relative file path
 * /param base the full path of a file on disk (not a directory)
 * /param file relative filepath that will be combined with the base
 * /return the full absolute path of the relative file argument
 */
fs::path combine_absolute_with_relative_path(const fs::path& baseFile, const fs::path& file);

/*!Create a relative path from an absolute path given a root dir
 * /param absPath the absolute path that will be converted to a relative path
 * /param root absolute path of the root directory
 * /return the relative path relative to the root path
 */
fs::path absolute_to_relative_path(const fs::path& absPath, const fs::path& root);

/*!Create an absolute path from a relative path
 * /param relPath the relative path that will be converted to an absolute path
 * /param base base directory for the relative reference
 * /return the absolute path
 */
fs::path relative_to_absolute_path(const fs::path& relPath, const fs::path& base);

/*!Create a full absolute path based on the base path and a relative file path
 * /param base the full path of a directory on disk (not a file)
 * /param file relative filepath that will be combined with the base
 * /return the full absolute path of the relative file argument
 */
fs::path combine_path(const fs::path& baseDir, const fs::path& file);

std::string read_as_text(const fs::path& filename);

#endif

std::string read_as_text(const char* filename);
std::string read_as_text(const std::string& filename);
std::string read_as_text(const std::istream& filestream);
void write_as_text(const fs::path& filename, std::string_view contents);

class Handle
{
public:
    Handle() = default;
    Handle(const fs::path& p, const char* mode)
    {
        ptr = std::fopen(p.string().c_str(), mode);
    }

    ~Handle()
    {
        if (ptr) {
            std::fclose(ptr);
        }
    }

    bool is_open() const
    {
        return ptr != nullptr;
    }

    std::FILE* get()
    {
        return ptr;
    }

    operator std::FILE*()
    {
        return ptr;
    }

private:
    std::FILE* ptr = nullptr;
};

class ScopedCurrentWorkingDirectory
{
public:
    ScopedCurrentWorkingDirectory(const fs::path& cwd);
    ~ScopedCurrentWorkingDirectory() noexcept;

private:
    fs::path _prevCwd;
};

}

namespace fmt {
template <>
struct formatter<fs::path>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const fs::path& p, FormatContext& ctx)
    {
        return format_to(ctx.begin(), "{}", p.string());
    }
};
}
